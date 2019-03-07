/*********************  P r o g r a m  -  M o d u l e ***********************/
/*!
 *        \file  m77_drv.c
 *
 *      \author  christophe.hannoyer@men.de
 *        $Date: 2009/07/10 15:56:21 $
 *    $Revision: 1.2 $
 *
 *       \brief  MDIS LL part for M77 QNX driver
 *
 *     Switches: -
 */
/*-------------------------------[ History ]---------------------------------
 *
 * $Log: m77_drv.c,v $
 * Revision 1.2  2009/07/10 15:56:21  channoyer
 * R: Incorrect swap mode for x86
 * M: Change swap condition
 *
 * Revision 1.1  2009/06/24 16:18:30  channoyer
 * Initial Revision
 *
 *---------------------------------------------------------------------------
 * (c) Copyright 2009 by MEN Mikro Elektronik GmbH, Nuremberg, Germany
 ****************************************************************************/

static const char RCSid[]="M77 - m77 low level driver $Id: m77_drv.c,v 1.2 2009/07/10 15:56:21 channoyer Exp $";

/* pass debug definitions to dbg.h */
#define DBG_MYLEVEL        m77Hdl->dbgLevel

#include <string.h>

#include <MEN/men_typs.h>
#include <MEN/maccess.h>
#include <MEN/oss.h>
#include <MEN/dbg.h>
#include <MEN/mdis_err.h>   /* mdis error numbers             */
#include <MEN/mbuf.h>       /* buffer lib functions and types */
#include <MEN/desc.h>
#include <MEN/mdis_api.h>   /* global set/getstat codes       */
#include <MEN/mdis_com.h>   /* info function      codes       */
#include <MEN/modcom.h>     /* ID PROM functions              */

#include <MEN/ll_defs.h>    /* low level driver definitions   */
#include <MEN/ll_entry.h>   /* low level driver entry struct  */

#include <MEN/m77_drv.h>    /* M77 defines + macros */

/*-----------------------------------------+
|  TYPEDEFS                                |
+------------------------------------------*/
typedef struct
{
    int32                 OwnMemSize;
    u_int32               dbgLevel;
    DBG_HANDLE           *dbgHdl;
    OSS_HANDLE           *osHdl;
    OSS_IRQ_HANDLE       *irqHdl;
    MACCESS               maUart;
    u_int32               useModulId;
    u_int32               unit_num;
    int32                 nbrOfChannels;
    u_int32               modId;
    OSS_SIG_HANDLE       *uartSig;
    MDIS_IDENT_FUNCT_TBL  idFuncTbl;
    M77_OPTIONS           m77opt[8];
    EVENT_CTRL_BLK_T      ecb;
} M77_HANDLE;


/*-----------------------------------------+
|  DEFINES & CONST                         |
+------------------------------------------*/
/* debug handle */
#define DBH        m77Hdl->dbgHdl

/*-----------------------------------------+
|  PROTOTYPES                              |
+------------------------------------------*/

static int32 M77_Init(        DESC_SPEC *descSpec, OSS_HANDLE     *osHdl,
                              MACCESS   *ma, OSS_SEM_HANDLE  *devSemHdl,
                              OSS_IRQ_HANDLE  *irqHdl, LL_HANDLE  **llHdlP );
static int32 M77_Exit(        LL_HANDLE **llHdlP );
static int32 M77_Read(        LL_HANDLE *llHdl,  int32 ch,  int32 *value );
static int32 M77_Write(       LL_HANDLE *llHdl,  int32 ch,  int32 value );
static int32 M77_SetStat(     LL_HANDLE *llHdl,  int32 ch,  int32 code,
                              int32 value );
static int32 M77_GetStat(     LL_HANDLE *llHdl,  int32 ch,  int32 code,
                              int32 *valueP );
static int32 M77_BlockRead(   LL_HANDLE *llHdl,  int32 ch,  void  *buf,
                              int32 size, int32 *nbrRdBytesP );
static int32 M77_BlockWrite(  LL_HANDLE *llHdl,  int32 ch,  void  *buf,
                              int32 size, int32 *nbrWrBytesP);
static int32 M77_Irq(         LL_HANDLE *llHdl );
static int32 M77_Info(        int32     infoType, ... );

/*****************************  M77_Ident  **********************************
 *
 *  Description:  Gets the pointer to ident string.
 *
 *
 *---------------------------------------------------------------------------
 *  Input......:  -
 *
 *  Output.....:  return  pointer to ident string
 *
 *  Globals....:  -
 ****************************************************************************/
static char* M77_Ident( void )
{
    return( (char*) RCSid );
}/* M77_Ident */

/**************************** M77_GetEntry *********************************
 *
 *  Description:  Dummy function to be conform with MDIS
 *                Gets the entry points of the low-level driver functions.
 *                Fill all with NULL pointer.
 *
 *
 *---------------------------------------------------------------------------
 *  Input......:  ---
 *
 *  Output.....:  drvP  pointer to the initialized structure
 *
 *  Globals....:  ---
 *
 ****************************************************************************/

extern void LL_GetEntry
(
    LL_ENTRY* drvP
)
{
    drvP->init        = M77_Init;
    drvP->exit        = M77_Exit;
    drvP->read        = M77_Read;
    drvP->write       = M77_Write;
    drvP->blockRead   = M77_BlockRead;
    drvP->blockWrite  = M77_BlockWrite;
    drvP->setStat     = M77_SetStat;
    drvP->getStat     = M77_GetStat;
    drvP->irq         = M77_Irq;
    drvP->info        = M77_Info;
}

static int32 M77_Init
(
    DESC_SPEC       *descSpec,
    OSS_HANDLE      *osHdl,
    MACCESS         *ma,
    OSS_SEM_HANDLE  *devSemHdl,
    OSS_IRQ_HANDLE  *irqHdl,
    LL_HANDLE      **llHdlP
)
{
    int8       zero = 0;
    M77_HANDLE  *m77Hdl;
    int32        retCode;
    u_int32     gotsize;
    DESC_HANDLE *descHdl;
    u_int32     dbgLevelDesc;
    u_int32     dbgLevelMbuf;
    int32        modIdMagic;
    int32        modId;
    int32       i;
    u_int32      descVal;
    u_int32     mode;

    retCode = DESC_Init( descSpec, osHdl, &descHdl );
    if( retCode )
    {
        return( retCode );
    }/* if */

    /*-------------------------------------+
    |  LL-HANDLE allocate and initialize   |
    +-------------------------------------*/
    m77Hdl = (M77_HANDLE*) OSS_MemGet( osHdl, sizeof(M77_HANDLE), &gotsize );
    if( m77Hdl == NULL )
    {
       *llHdlP = 0;
       return( ERR_OSS_MEM_ALLOC );
    }/*if*/

    /* set return ll-handle */
    *llHdlP = (LL_HANDLE*) m77Hdl;

    /* fill turkey with 0 */
    OSS_MemFill( osHdl, gotsize, (char*) m77Hdl, zero );

    /* fill up the turkey */
    m77Hdl->OwnMemSize = gotsize;
    m77Hdl->osHdl      = osHdl;
    m77Hdl->maUart     = *ma;
    m77Hdl->irqHdl     = irqHdl;

    /*------------------------------+
    |  init id function table       |
    +------------------------------*/
    /* drivers ident function */
    m77Hdl->idFuncTbl.idCall[0].identCall = M77_Ident;
    /* libraries ident functions */
    m77Hdl->idFuncTbl.idCall[1].identCall = MBUF_Ident;
    m77Hdl->idFuncTbl.idCall[2].identCall = DESC_Ident;
    m77Hdl->idFuncTbl.idCall[3].identCall = OSS_Ident;
    /* terminator */
    m77Hdl->idFuncTbl.idCall[4].identCall = NULL;

    /* prepare debugging */
    DBG_MYLEVEL = DBG_ALL;        /* say: all debug ON */
    DBGINIT((NULL,&DBH));

    /*-------------------------------------+
    |  get DEBUG params                    |
    +-------------------------------------*/
    /* DEBUG_LEVEL_DESC */
    retCode = DESC_GetUInt32( descHdl,
                              DBG_OFF,
                              &dbgLevelDesc,
                              "DEBUG_LEVEL_DESC",
                              NULL );
    if( DESC_FATAL(retCode) ) goto CLEANUP;
    DESC_DbgLevelSet(descHdl, dbgLevelDesc);
    retCode = ERR_SUCCESS;

    /* DEBUG_LEVEL_MBUF */
    retCode = DESC_GetUInt32( descHdl,
                              DBG_OFF,
                              &dbgLevelMbuf,
                              "DEBUG_LEVEL_MBUF",
                              NULL );
    if( DESC_FATAL(retCode) ) goto CLEANUP;
    retCode = ERR_SUCCESS;

    /* DEBUG_LEVEL */
    retCode = DESC_GetUInt32( descHdl,
                              DBG_OFF,
                              &m77Hdl->dbgLevel,
                              "DEBUG_LEVEL",
                              NULL );
    if( DESC_FATAL(retCode) ) goto CLEANUP;
    retCode = ERR_SUCCESS;

    retCode = DESC_GetUInt32( descHdl,
                              M_BUF_USRCTRL,
                              &mode,
                              "RD_BUF/MODE",
                              0 );
    if( DESC_FATAL(retCode) ) goto CLEANUP;
    retCode = 0;

    /*-------------------------------------+
    |  descriptor - use module id ?        |
    +-------------------------------------*/
    retCode = DESC_GetUInt32( descHdl,
                              1,
                              &m77Hdl->useModulId,
                              "ID_CHECK",
                              NULL );
    if( DESC_FATAL(retCode) ) goto CLEANUP;
    retCode = 0;

    if( m77Hdl->useModulId)
    {
        /* get id from M-Module eeprom */
        modIdMagic = m_read((U_INT32_OR_64)m77Hdl->maUart, 0);
        modId      = m_read((U_INT32_OR_64)m77Hdl->maUart, 1);
        if( modIdMagic != MOD_ID_MAGIC )
        {
             DBGWRT_ERR((DBH,
                         "*** %s: m_read() id - illegal magic word 0x%x 0x%x 0x%x\n",
                         __FUNCTION__, (int) modIdMagic, MOD_ID_MAGIC,
                         (int) modId ));
             retCode = ERR_LL_ILL_ID;
             goto CLEANUP;
        }/*if*/

        if( modId != MOD_ID_M77 &&
            modId != MOD_ID_M45N &&
            modId != MOD_ID_M69N ) {
            DBGWRT_ERR((DBH, "*** %s:  m_read() id - illegal module id\n",
                        __FUNCTION__ ));
            retCode = ERR_LL_ILL_ID;
            goto CLEANUP;
        }/*if*/
        m77Hdl->modId = modId;
    } else {
        m77Hdl->modId = MOD_ID_M77;
    }/*if*/

    /*----------------------------+
    | set M-Module specific info: |
    | channels, devName           |
    | save in driver structure    |
    +----------------------------*/
    switch ( m77Hdl->modId ){
    case MOD_ID_M45N:
        m77Hdl->nbrOfChannels = M45_MAX_CH;
        break;
    case MOD_ID_M69N:
        m77Hdl->nbrOfChannels = M69_MAX_CH;
        break;
    default:
        m77Hdl->nbrOfChannels = M77_MAX_CH;
        break;
    }

    retCode = DESC_GetUInt32( descHdl,
                              1,
                              &m77Hdl->unit_num,
                              "UNIT_NUMBER",
                              NULL );
    if( DESC_FATAL(retCode) ) goto CLEANUP;
    retCode = 0;

    /*----------------------------------------+
    |   channel specific descriptor entries   |
    +----------------------------------------*/
    for (i = 0; i <  m77Hdl->nbrOfChannels; i++) {

        if( (m77Hdl->modId == MOD_ID_M45N) && (i > 3) ) {
            m77Hdl->m77opt[i].offset = M45N_CTRL_REG_BLOCK_SIZE +
                ((i - 4) * 0x10) ;
        } else {
            m77Hdl->m77opt[i].offset = i * 0x10;
        }

        /* get SIO_HW_OPTS */
        retCode = DESC_GetUInt32( descHdl,
                                  M77_DEF_SIO_HW_OPTS,
                                  &descVal,
                                  "CHANNEL_%d/SIO_HW_OPTS",
                                   i);

        if( DESC_FATAL(retCode) )
        {
            DBGWRT_ERR(( DBH,
                         " *** m77ReadChanDesc: DESC_GetUInt32 SIO_HW_OPTS\n"));
            goto CLEANUP;
        }

        m77Hdl->m77opt[i].sio_hw_opts = (u_int16) descVal;

        /* get FIOBAUDRATE */
        retCode = DESC_GetUInt32( descHdl,
                                  M77_DEFAULT_BAUD,
                                  &descVal,
                                  "CHANNEL_%d/FIOBAUDRATE",
                                   i);

        if( DESC_FATAL(retCode) )
        {
            DBGWRT_ERR(( DBH,
                         " *** m77ReadChanDesc: DESC_GetUInt32 FIOBAUDRATE\n"));
            goto CLEANUP;
        }

        m77Hdl->m77opt[i].baudrate = descVal;

        /* get driver internal Rx buffer size */
        retCode = DESC_GetUInt32( descHdl,
                                  M77_DEF_RX_BUFF_SIZE,
                                  &descVal,
                                  "CHANNEL_%d/DRV_RX_BUFF_SIZE",
                                   i);

        if( DESC_FATAL(retCode) )
        {
            DBGWRT_ERR(( DBH,
                         " *** m77ReadChanDesc: DESC_GetUInt32 DRV_RX_BUFF_SIZE\n"));
            goto CLEANUP;
        }

        m77Hdl->m77opt[i].rdBufSize = (int) descVal;

        /* get driver internal Tx buffer size */
        retCode = DESC_GetUInt32( descHdl,
                                  M77_DEF_TX_BUFF_SIZE,
                                  &descVal,
                                  "CHANNEL_%d/DRV_TX_BUFF_SIZE",
                                   i);

        if( DESC_FATAL(retCode) )
        {
            DBGWRT_ERR(( DBH,
                         " *** m77ReadChanDesc: DESC_GetUInt32 DRV_TX_BUFF_SIZE\n"));
            goto CLEANUP;
        }

        m77Hdl->m77opt[i].wrtBufSize = (int) descVal;

        /* get TX_FIFO_LEVEL */
        retCode = DESC_GetUInt32( descHdl,
                                  TX_FIFO_LEVEL_DEFAULT,
                                  &descVal,
                                  "CHANNEL_%d/TX_FIFO_LEVEL",
                                   i);

        if( DESC_FATAL(retCode) )
        {
            DBGWRT_ERR( ( DBH, " *** m77ReadChanDesc: DESC_GetUInt32 TX_FIFO_LEVEL\n"));
            goto CLEANUP;
        }

        /* check if descVal is in range */
        if (descVal >= M77_FIFO_SIZE) {
            DBGWRT_ERR( ( DBH, " *** m77ReadChanDesc: DESC_GetUInt32 TX_FIFO_LEVEL "
            "not in range\n"));
            retCode = ERR_LL_ILL_PARAM;
            goto CLEANUP;
        }
        m77Hdl->m77opt[i].transmit_level = (u_int16) descVal;

        /* get RX_FIFO_LEVEL */
        retCode = DESC_GetUInt32( descHdl,
                                  RX_FIFO_LEVEL_DEFAULT,
                                  &descVal,
                                  "CHANNEL_%d/RX_FIFO_LEVEL",
                                   i);

        if( DESC_FATAL(retCode) )
        {
            DBGWRT_ERR( ( DBH, " *** m77ReadChanDesc: DESC_GetUInt32 RX_FIFO_LEVEL\n"));
            goto CLEANUP;
        }

        /* a value of 0 causes an interrupt which can not be cleared => min = 1 */
        if (descVal < 1 || descVal >= M77_FIFO_SIZE) {
            DBGWRT_ERR( ( DBH, " *** m77ReadChanDesc: DESC_GetUInt32 RX_FIFO_LEVEL "
            "not in range\n"));
            retCode = ERR_LL_ILL_PARAM;
            goto CLEANUP;
        }
        m77Hdl->m77opt[i].receive_level = (u_int16) descVal;

        /* get NO_FIFO */
        retCode = DESC_GetUInt32( descHdl,
                                  0, /* FIFO enable */
                                  &descVal,
                                  "CHANNEL_%d/NO_FIFO",
                                   i);

        if( DESC_FATAL(retCode) )
        {
            DBGWRT_ERR( ( DBH, " *** m77ReadChanDesc: DESC_GetUInt32 NO_FIFO\n"));
            goto CLEANUP;
        }

        if (descVal) {
            m77Hdl->m77opt[i].fifo_enabled = FALSE;
        }
        else {
            m77Hdl->m77opt[i].fifo_enabled = TRUE;
        }

        if( m77Hdl->modId == MOD_ID_M77 )
        {
            /* get PHYS_INT */
            retCode = DESC_GetUInt32( descHdl,
                                      M77_RS422_FD,
                                      &descVal,
                                      "CHANNEL_%d/PHYS_INT",
                                      i);

            if( DESC_FATAL(retCode) )
            {
                DBGWRT_ERR( ( DBH, " *** m77ReadChanDesc: DESC_GetUInt32 PHYS_INT\n"));
                goto CLEANUP;
            }

            m77Hdl->m77opt[i].physInt = (u_int16) descVal;

            /* get ECHO_SUPPRESS */
            retCode = DESC_GetUInt32( descHdl,
                                      1, /* echo suppression enabled */
                                      &descVal,
                                      "CHANNEL_%d/ECHO_SUPPRESS",
                                       i);
            if( DESC_FATAL(retCode) )
            {
                DBGWRT_ERR( ( DBH, " *** m77ReadChanDesc: DESC_GetUInt32 ECHO_SUPPRESS\n"));
                goto CLEANUP;
            }
            if (descVal) {
                m77Hdl->m77opt[i].echo_suppress = TRUE;
            }
            else {
                m77Hdl->m77opt[i].echo_suppress = FALSE;
            }
        } else {
            m77Hdl->m77opt[i].physInt = M77_RS232;
        }

        /* get HANDSHAKE_HIGH_FIFO_LEVEL */
        retCode = DESC_GetUInt32( descHdl,
                                  M77_DEF_FCH,
                                  &descVal,
                                  "CHANNEL_%d/HANDSHAKE_HIGH_FIFO_LEVEL",
                                   i);
        if( DESC_FATAL(retCode) )
        {
            DBGWRT_ERR( ( DBH, " *** m77ReadChanDesc: DESC_GetUInt32 "
                                "HANDSHAKE_HIGH_FIFO_LEVEL\n"));
            goto CLEANUP;
        }
        m77Hdl->m77opt[i].hsHighFifoLevel = descVal;

        /* get HANDSHAKE_LOW_FIFO_LEVEL */
        retCode = DESC_GetUInt32( descHdl,
                                  M77_DEF_FCL,
                                  &descVal,
                                  "CHANNEL_%d/HANDSHAKE_LOW_FIFO_LEVEL",
                                   i);
        if( DESC_FATAL(retCode) )
        {
            DBGWRT_ERR( ( DBH, " *** m77ReadChanDesc: DESC_GetUInt32 "
                                "HANDSHAKE_LOW_FIFO_LEVEL\n"));
            goto CLEANUP;
        }
        m77Hdl->m77opt[i].hsLowFifoLevel  = descVal;

        /* get HANDSHAKE_XONXOFF */
        retCode = DESC_GetUInt32( descHdl,
                                  M77_MODEM_HS_NONE, /* no handshake */
                                  &descVal,
                                  "CHANNEL_%d/HANDSHAKE_XONXOFF",
                                   i);
        if( DESC_FATAL(retCode) )
        {
            DBGWRT_ERR( ( DBH, " *** m77ReadChanDesc: DESC_GetUInt32 "
                                "HANDSHAKE_XONXOFF\n"));
            goto CLEANUP;
        }
        m77Hdl->m77opt[i].hsMode |= descVal ? M77_MODEM_HS_XONXOFF : 0;

        /* M45N/M69N: get HANDSHAKE settings */
        if( m77Hdl->modId == MOD_ID_M45N ||
            m77Hdl->modId == MOD_ID_M69N )
        {
            retCode = DESC_GetUInt32( descHdl,
                                      M77_MODEM_HS_NONE, /* no handshake */
                                      &descVal,
                                      "CHANNEL_%d/HANDSHAKE_AUTO_CTS",
                                       i);
            if( DESC_FATAL(retCode) )
            {
                DBGWRT_ERR( ( DBH, " *** m77ReadChanDesc: DESC_GetUInt32 "
                                    "HANDSHAKE_AUTO_CTS\n"));
                goto CLEANUP;
            }
            m77Hdl->m77opt[i].hsMode |= descVal ? M77_MODEM_HS_AUTO_CTS : 0;

            retCode = DESC_GetUInt32( descHdl,
                                      M77_MODEM_HS_NONE, /* no handshake */
                                      &descVal,
                                      "CHANNEL_%d/HANDSHAKE_AUTO_DSR",
                                       i);
            if( DESC_FATAL(retCode) )
            {
                DBGWRT_ERR( ( DBH, " *** m77ReadChanDesc: DESC_GetUInt32 "
                                    "HANDSHAKE_AUTO_DSR\n"));
                goto CLEANUP;
            }
            m77Hdl->m77opt[i].hsMode |= descVal ? M77_MODEM_HS_AUTO_DSR : 0;

            /* full handshaking only on two channels of M45N/M69N */
            if( (m77Hdl->modId == MOD_ID_M45N && i <= 1 )||
                (m77Hdl->modId == MOD_ID_M69N && i >  1 ) )
            {
                retCode = DESC_GetUInt32( descHdl,
                                          M77_MODEM_HS_NONE, /* no handshake */
                                          &descVal,
                                          "CHANNEL_%d/HANDSHAKE_AUTO_RTS",
                                           i);
                if( DESC_FATAL(retCode) )
                {
                    DBGWRT_ERR( ( DBH, " *** m77ReadChanDesc: DESC_GetUInt32 "
                                        "HANDSHAKE_AUTO_RTS\n"));
                    goto CLEANUP;
                }
                m77Hdl->m77opt[i].hsMode |= descVal ? M77_MODEM_HS_AUTO_RTS : 0;

                retCode = DESC_GetUInt32( descHdl,
                                          M77_MODEM_HS_NONE, /* no handshake */
                                          &descVal,
                                          "CHANNEL_%d/HANDSHAKE_AUTO_DTR",
                                           i);
                if( DESC_FATAL(retCode) )
                {
                    DBGWRT_ERR( ( DBH, " *** m77ReadChanDesc: DESC_GetUInt32 "
                                        "HANDSHAKE_AUTO_DTR\n"));
                    goto CLEANUP;
                }
                m77Hdl->m77opt[i].hsMode |= descVal ? M77_MODEM_HS_AUTO_DSR : 0;
            }
        }

        /* M45N: get TRISTATE settings */
        if( m77Hdl->modId == MOD_ID_M45N )
        {
            retCode = DESC_GetUInt32( descHdl,
                                      0, /* normal operation */
                                      &descVal,
                                      "CHANNEL_%d/TRISTATE",
                                      i);
            if( DESC_FATAL(retCode) )
            {
                DBGWRT_ERR( ( DBH, " *** m77ReadChanDesc: DESC_GetUInt32 "
                              "TRISTATE\n"));
                goto CLEANUP;
            }
            m77Hdl->m77opt[i].tristate = descVal ? 1 : 0;
        }
    }

    switch ( m77Hdl->modId ){
    case MOD_ID_M45N:
        MWRITE_D16(m77Hdl->maUart, 0x48, 0x3);
        MWRITE_D16(m77Hdl->maUart, 0xc8, 0x3);
        break;
    case MOD_ID_M69N:
        break;
    default:
        MWRITE_D16(m77Hdl->maUart, 0x48, 0x7);
        break;
    }
    retCode = 0;

    DBGWRT_1((DBH, "LL - M77_Init\n" )  );

    DESC_Exit( &descHdl );
    return( retCode );

 CLEANUP:
    DESC_Exit( &descHdl );

    OSS_MemFree(osHdl, m77Hdl, m77Hdl->OwnMemSize);

    return( retCode );
} /* M77_Init */

static int32 M77_Exit
(
     LL_HANDLE **llHdlP
)
{
    M77_HANDLE*       m77Hdl;

    m77Hdl = (M77_HANDLE*) *llHdlP;

    return( ERR_SUCCESS );
} /* M77_Exit */

static int32 M77_Read
(
    LL_HANDLE *llHdl,
    int32      ch,
    int32     *valueP
)
{
    M77_HANDLE*       m77Hdl;

    m77Hdl = (M77_HANDLE*) llHdl;

    *valueP = MREAD_D32(m77Hdl->maUart, ch);

    return( ERR_SUCCESS );
} /* M77_Read */

static int32 M77_Write
(
    LL_HANDLE *llHdl,
    int32      ch,
    int32      value
)
{
    M77_HANDLE*       m77Hdl;

    m77Hdl = (M77_HANDLE*) llHdl;

    return( ERR_SUCCESS );
} /* M77_Write */

static int32 M77_SetStat
(
    LL_HANDLE *llHdl,
    int32      code,
    int32      ch,
    int32      value
)
{
    int32        retCode;
    M77_HANDLE*       m77Hdl;

    m77Hdl = (M77_HANDLE*) llHdl;

    retCode = ERR_SUCCESS;

    switch(code)
    {
        /*--------------------------+
        |  debug level              |
        +--------------------------*/
        case M_LL_DEBUG_LEVEL:
            m77Hdl->dbgLevel = value;
            break;
        /*--------------------------+
        |  enable interrupts        |
        +--------------------------*/
        case M_MK_IRQ_ENABLE:
            break;

        /*--------------------------+
        |  (unknown)                |
        +--------------------------*/
        default:

            retCode = ERR_LL_UNK_CODE;
    }/*switch*/

    return( retCode );
} /* M77_SetStat */

static int32 M77_GetStat
(
    LL_HANDLE *llHdl,
    int32 code,
    int32 ch,
    int32 *valueP
)
{
    M77_HANDLE*       m77Hdl;
    M_SETGETSTAT_BLOCK *blk = (M_SG_BLOCK *)valueP;

    m77Hdl = (M77_HANDLE*) llHdl;

    switch(code)
    {
        /*------------------+
        |  get ch count     |
        +------------------*/
        case M_LL_CH_NUMBER:
            *valueP = m77Hdl->nbrOfChannels;
            break;

        /*------------------+
        |  id check enabled |
        +------------------*/
        case M_LL_ID_CHECK:
            *valueP = m77Hdl->useModulId;
            break;

        /*------------------+
        |  debug level       |
        +------------------*/
        case M_LL_DEBUG_LEVEL:
            *valueP = m77Hdl->dbgLevel;
            break;

        /*--------------------+
        |  ident table        |
        +--------------------*/
        case M_MK_BLK_REV_ID:
           *valueP = (int32)&m77Hdl->idFuncTbl;
           break;

        /* ------ special setstat codes --------- */

        case M77_GET_MODID:
            *valueP = m77Hdl->modId;
            break;

        case M77_GET_UNIT_NUM:
            *valueP = m77Hdl->unit_num;
            break;

        case M77_GET_SW_MODE:
#if (defined(_BIG_ENDIAN_) && !defined(MAC_BYTESWAP)) || (defined(_LITTLE_ENDIAN_) && defined(MAC_BYTESWAP))
            *valueP = 1;
#else
            *valueP = 0;
#endif
            break;

        /*--------------------------+
        |                           |
        +--------------------------*/
        case M77_GET_BLK_OPTIONS:
            memcpy(blk->data, &(m77Hdl->m77opt[ch]), blk->size);
            break;

        default:
            break;
    }

    return( ERR_SUCCESS );
} /* M77_GetStat */

static int32 M77_BlockRead
(
    LL_HANDLE *llHdl,
    int32      ch,
    void      *buf,
    int32      size,
    int32     *nbrRdBytesP
)
{
    M77_HANDLE*       m77Hdl;

    m77Hdl = (M77_HANDLE*) llHdl;

    return( ERR_SUCCESS );
} /* M77_BlockRead */

static int32 M77_BlockWrite
(
    LL_HANDLE *llHdl,
    int32      ch,
    void      *buf,
    int32      size,
    int32     *nbrWrBytesP
)
{
    M77_HANDLE*       m77Hdl;

    m77Hdl = (M77_HANDLE*) llHdl;

    return( ERR_SUCCESS );
} /* M77_BlockWrite */

static int32 M77_Irq
(
     LL_HANDLE *llHdl
)
{
    M77_HANDLE*       m77Hdl;

    m77Hdl = (M77_HANDLE*) llHdl;

    IDBGWRT_1((DBH, ">> m77_irq\n" )  );

    return( ERR_SUCCESS );
} /* M77_Irq */

static int32 M77_Info
(
    int32 infoType,
    ...
)
{
    int32    error = ERR_SUCCESS;
    va_list  argptr;
    u_int32 *nbrOfAddrSpaceP;
    u_int32 *addrModeP;
    u_int32 *dataModeP;
    u_int32 *addrSizeP;
    u_int32 *useIrqP;
    u_int32  addrSpaceIndex;

    va_start(argptr, infoType );

    switch(infoType) {
        case LL_INFO_HW_CHARACTER:
          addrModeP  = va_arg( argptr, u_int32* );
          dataModeP  = va_arg( argptr, u_int32* );
          *addrModeP = MDIS_MA08;
          *dataModeP = MDIS_MD08;
          break;

        case LL_INFO_ADDRSPACE_COUNT:
          nbrOfAddrSpaceP = va_arg( argptr, u_int32* );
          *nbrOfAddrSpaceP = 1;
          break;

        case LL_INFO_ADDRSPACE:
          addrSpaceIndex = va_arg( argptr, u_int32 );
          addrModeP  = va_arg( argptr, u_int32* );
          dataModeP  = va_arg( argptr, u_int32* );
          addrSizeP  = va_arg( argptr, u_int32* );

          switch( addrSpaceIndex )
          {
              case 0:
                *addrModeP = MDIS_MA08;
                *dataModeP = MDIS_MD16;
                *addrSizeP = 0x100;
                break;

              default:
                 error = ERR_LL_ILL_PARAM;
          }/*switch*/
          break;

        case LL_INFO_IRQ:
          useIrqP  = va_arg( argptr, u_int32* );
          *useIrqP = TRUE;
          break;

        case LL_INFO_LOCKMODE:
        {
            u_int32 *lockModeP = va_arg(argptr, u_int32*);

            *lockModeP = LL_LOCK_CALL;
        }
        break;

        default:
          error = ERR_LL_ILL_PARAM;
    }

    va_end(argptr);
    return(error);
} /* M77_Info */
