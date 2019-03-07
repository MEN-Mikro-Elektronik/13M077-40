/*********************  P r o g r a m  -  M o d u l e ***********************/
/*!
 *        \file  options.c
 *
 *      \author  christophe.hannoyer@men.de
 *        $Date: 2009/06/25 10:56:48 $
 *    $Revision: 1.1 $
 *
 *       \brief  Parses the driver's command-line arguments
 *
 *     Switches: -
 */
/*-------------------------------[ History ]---------------------------------
 *
 * $Log: options.c,v $
 * Revision 1.1  2009/06/25 10:56:48  channoyer
 * Initial Revision
 *
 *---------------------------------------------------------------------------
 * (c) Copyright 2009 by MEN Mikro Elektronik GmbH, Nuremberg, Germany
 ****************************************************************************/

#include "externs.h"

unsigned
options(int argc, char *argv[]) {
    int opt, numports = 0;
    char *cp;
    size_t length;
    void *link;
    u_int32 unit;
    static TTYINIT devinit ={
        0, 1, 0, -1,
        2048, 2048, 256,
        0, 0, 0, 0, 0, 0, 0,
        "/dev/ser", 0, 0 };
    u_int16 drvMode = 0xFF;

    length = sizeof( MODULE_M77 );
    G_modM77P = malloc( length );
    if( G_modM77P == NULL ) {
        return(0);
    }

    // Initialize the devinit to raw mode
    ttc(TTC_INIT_RAW, &devinit, 0);

    sys_ttyinit(&devinit);

    unit = 1;
    while(optind < argc) {
        // Process dash options.
        while((opt =
               getopt(argc, argv, IO_CHAR_SERIAL_OPTIONS "c:D:m:u:")) != -1) {
            switch(ttc(TTC_SET_OPTION, &devinit, opt)) {
            case 'c':
                devinit.clk = strtoul(optarg, &optarg, 0);
                if((cp = strchr(optarg, '/'))) {
                    devinit.div = strtoul(cp + 1, NULL, 0);
                }
                break;
            case 'D':
                memset( G_modM77P, 0, length);
                length = strlen(optarg);
                if( length < MAX_DEV_NAME ) {
                    strncpy(G_modM77P->mdis_dev_name, optarg, length+1);
                    link = query_mdis_device( G_modM77P );
                } else {
                    fprintf(stderr,"MDIS device name too long.\n");
                }
                break;
            case 'm':
                if (G_modM77P->modId != MOD_ID_M77) {
                    fprintf(stderr,"Option only for M77.\n");
                } else {
                    drvMode = strtoul(optarg, NULL, 0);
                    switch (drvMode) {
                    case 1:
                    case 2:
                    case 3:
                    case 4:
                    case 7:
                        break;
                    default:
                        fprintf(stderr,
                                "Invalid mode %d. Must be 1, 2, 3, 4 or 7\n",
                                drvMode);
                        drvMode = 0xFF;
                        break;
                    }
                }
                break;
            case 'u':
                G_modM77P->unit_num = strtoul(optarg, NULL, 0);
                break;
            }
        }

        /* Process "auto" parameter. */
        while(optind < argc  &&  *(optarg = argv[optind]) != '-') {

            if( strncmp( optarg, "auto", 4 ) == 0 ) {
                if( numports < G_modM77P->nbrOfChannels ) {
                    devinit.port =
                        (unsigned long int) G_modM77P->m77opt[numports].ma;
                    devinit.port += G_modM77P->swMode;
                    devinit.intr = G_modM77P->mdis.irqVector;

                    if(drvMode != 0xFF) {
                        G_modM77P->m77opt[numports].physInt = drvMode;
                    }

                    create_device(&devinit, numports);
                    ++numports;
                } else {
                    fprintf(stderr, "Unexpected number of auto parameter\n");
                    numports = 0;
                    goto CLEANUP;
                }
            } else {
                fprintf(stderr, "Unexpected option %s\n", optarg);
                numports = 0;
                goto CLEANUP;
            }

            ++optind;
        }

    }

    if(numports == 0) {
        while( numports < G_modM77P->nbrOfChannels ) {
            devinit.port = (unsigned long int) G_modM77P->m77opt[numports].ma;
            devinit.port += G_modM77P->swMode;
            devinit.intr = G_modM77P->mdis.irqVector;

            if(drvMode != 0xFF) {
                G_modM77P->m77opt[numports].physInt = drvMode;
            }

            create_device(&devinit, numports);
            ++numports;
        }
    }

 CLEANUP:
    return(numports);
}

