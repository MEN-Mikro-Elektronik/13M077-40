/*********************  P r o g r a m  -  M o d u l e ***********************/
/*!
 *        \file  query_mdisdev.c
 *
 *      \author  christophe.hannoyer@men.de
 *        $Date: 2009/07/10 15:53:47 $
 *    $Revision: 1.2 $
 *
 *       \brief  Queries the default devices.
 *
 *               Get devices informations from the descriptor
 *
 *     Switches: -
 */
/*-------------------------------[ History ]---------------------------------
 *
 * $Log: query_mdisdev.c,v $
 * Revision 1.2  2009/07/10 15:53:47  channoyer
 * R: QNX 6.4 warning: pointer targets differ in signedness
 * M: change variable type
 *
 * Revision 1.1  2009/06/24 16:24:17  channoyer
 * Initial Revision
 *
 *---------------------------------------------------------------------------
 * (c) Copyright 2009 by MEN Mikro Elektronik GmbH, Nuremberg, Germany
 ****************************************************************************/

#include "externs.h"

/*-----------------------------------------+
|  TYPDEFS                                 |
+------------------------------------------*/

/*-----------------------------------------+
|  DEFINES & CONST                         |
+------------------------------------------*/

/*-----------------------------------------+
|  GLOBALS                                 |
+------------------------------------------*/

/*-----------------------------------------+
|  PROTOTYPES                              |
+------------------------------------------*/

void *IntThread( void *arg );

void *
query_mdis_device()
{
    int32       channel;
    M_SETGETSTAT_BLOCK blk;
    int32 length;
    int fd, rv=-1, rc, coid;
    MDIS_OPEN_DEVICE_DATA moData;
    MDIS_OPEN_DEVICE_RETURN *r = (MDIS_OPEN_DEVICE_RETURN*)&moData;
    MDIS_MSGHEADER   h;
    iov_t            siov;
    pthread_t intThread = NULL;
    int32     value;

    strncpy( moData.devName, G_modM77P->mdis_dev_name, sizeof(moData.devName) );
    moData.pid = getpid();
    moData.tid = pthread_self();

    /* open global MDIS device */
    if( (fd = open(MDIS_DEV_NAME, O_RDWR)) < 0 ) {
        goto CLEANUP;
    }

    /* ask driver to open MDIS device */
    if(( rv = devctl( fd, MDIS_DEVCTL_M_OPEN, &moData, sizeof(moData), &rc ))){
        goto CLEANUP;
    }

    /* check return code from MDIS_OpenDevice() */
    if( rc ) {
        goto CLEANUP;
    }

    /* create connection to MDIS kernel thread */
    if( (coid = ConnectAttach( 0, r->mkPid, r->mkCid,
                                 _NTO_SIDE_CHANNEL, 0 )) == -1) {
        goto CLEANUP;
    }

    if( (G_modM77P->mp = malloc( sizeof( MDIS_PATH_QNX ))) == NULL ) {
        goto CLEANUP;
    }

    G_modM77P->mp->path = r->path;
    G_modM77P->mp->coid = coid;
    G_modM77P->mp->fd   = fd;

    length = sizeof(MDIS_NATDEV_RETURN);
    MDIS_SETMSG( &h, G_modM77P->mp->path, MDIS_MSGID_NATGET, 0, 0, length );
    SETIOV( &siov, &h, sizeof(h) );

    if( (rv = MsgSendvs( G_modM77P->mp->coid, &siov, 1,
                         &G_modM77P->mdis, length )) >
        ERR_OS )
        errno = rv;

    M_setstat((int)G_modM77P->mp, M_MK_IRQ_ENABLE, 1);

    if( M_getstat((int)G_modM77P->mp, M77_GET_MODID, &value) < 0) {
        fprintf(stderr,"M_getstat failed\n" );
        goto CLEANUP;
    }
    G_modM77P->modId = value;

    if( M_getstat((int)G_modM77P->mp, M_LL_CH_NUMBER,
                  &G_modM77P->nbrOfChannels) < 0) {
        fprintf(stderr,"M_getstat failed\n" );
        goto CLEANUP;
    }

    if( M_getstat((int)G_modM77P->mp, M77_GET_UNIT_NUM,
                  &value) < 0) {
        fprintf(stderr,"M_getstat failed\n" );
        goto CLEANUP;
    }
    G_modM77P->unit_num = value;

    if( M_getstat((int)G_modM77P->mp, M77_GET_SW_MODE,
                  &value) < 0) {
        fprintf(stderr,"M_getstat failed\n" );
        goto CLEANUP;
    }
    G_modM77P->swMode = value;

    /*----------------------------------------+
    |   channel specific descriptor entries   |
    +----------------------------------------*/
    for (channel = 0; channel <  G_modM77P->nbrOfChannels; channel++) {
        if( M_setstat((int)G_modM77P->mp, M_MK_CH_CURRENT, channel) < 0) {
            fprintf(stderr,"M_setstat failed\n" );
            goto CLEANUP;
        }

        blk.size = sizeof(G_modM77P->m77opt[0]);
        blk.data = (void*)&G_modM77P->m77opt[channel];
        M_getstat((int)G_modM77P->mp, M77_GET_BLK_OPTIONS, (int32*) &blk);

        G_modM77P->m77opt[channel].ma =
            (MACCESS) (G_modM77P->m77opt[channel].offset +
                       G_modM77P->mdis.physAddr);

    }

    G_modM77P->module_port =
        (uintptr_t)mmap_device_memory(NULL, 0x100,
                                      PROT_READ|PROT_WRITE|PROT_NOCACHE, 0,
                                      (unsigned)(G_modM77P->mdis.physAddr));
    if(G_modM77P->module_port == -1) {
        fprintf(stderr, "error %d\n", errno);
        exit( EXIT_FAILURE );
    }
    G_modM77P->module_port += G_modM77P->swMode;

    // start interrupt handler thread
    if( pthread_create( &intThread, NULL, &IntThread, (void *)G_modM77P )
        != EOK ){
        printf("Couldn't start intThread\n");
        exit( EXIT_FAILURE );
    }

 CLEANUP:
    return NULL;
}

/********************************* IntThread *****************************
 *
 *  Description: Thread to handle/dispatch ALL VME interrupts
 *
 *---------------------------------------------------------------------------
 *  Input......:
 *  Output.....: -
 *  Globals....:
 ****************************************************************************/
void *IntThread( void *arg )
{
    struct sigevent *event_intr/* , event */;
    struct _pulse pulse;
    MDIS_PATH_QNX *mp = G_modM77P->mp;
    MDIS_MSGHEADER   h;
    int rv=-1;
    iov_t            siov[2];
    IRQ_THREAD      irqThread;
    struct dev_list        *list_curr;
    u_int8      uartNum;
    u_int8      pendIrq;

    /* create a channel on which we can receive the pulse */
    irqThread.cid = ChannelCreate(0);
    irqThread.tid = getpid();

    if( irqThread.cid<0 /* || irqThread.coid<0 */ ){
        perror("Cannot create channel");
        exit( EXIT_FAILURE );
    }

    MDIS_SETMSG( &h, mp->path, MDIS_MSGID_NATIRQ, 0, 0, sizeof(irqThread) );
    SETIOV( &siov[0], &h, sizeof(h) );
    SETIOV( &siov[1], &irqThread, sizeof(irqThread) );

    if( (rv = MsgSendvs( mp->coid, siov, 2, 0, 0)) > ERR_OS )
        errno = rv;

    while( 1 ){
        int rcvid;

        rcvid = MsgReceive( irqThread.cid, &pulse, sizeof(pulse), NULL );

        if( rcvid < 0 ) {
            perror("MsgReceivePulse");

        }else {
            uartNum = 0;
            list_curr = G_devicesP;
            while ( list_curr != NULL ) {

                pendIrq = MREAD_D8(G_modM77P->module_port, 0x48 +
                                   (M45N_CTRL_REG_BLOCK_SIZE * uartNum) );

                if ( pendIrq & M77_IRQ_CLEAR ) {

                    event_intr = ser_intr(list_curr, 0);
                    if (event_intr != NULL) {
                        MsgSendPulse(event_intr->sigev_coid,
                                     event_intr->sigev_priority,
                                     event_intr->sigev_code, 0);
                    }

                    MWRITE_D8(G_modM77P->module_port, 0x48 +
                              (M45N_CTRL_REG_BLOCK_SIZE * uartNum), pendIrq);
                }
                list_curr = list_curr->next;
                uartNum++;
            }

            MsgReply(rcvid, 0, NULL, 0);

        }
    }

    return(0);
}
