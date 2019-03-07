/*********************  P r o g r a m  -  M o d u l e ***********************/
/*!
 *        \file  m77_devctl.c
 *
 *      \author  christophe.hannoyer@men.de
 *        $Date: 2009/07/10 16:40:40 $
 *    $Revision: 1.1 $
 *
 *       \brief  M45N/M69N/M77 specific code
 *
 *     Switches: -
 */
/*-------------------------------[ History ]---------------------------------
 *
 * $Log: m77_devctl.c,v $
 * Revision 1.1  2009/07/10 16:40:40  channoyer
 * Initial Revision
 *
 *---------------------------------------------------------------------------
 * (c) Copyright 2009 by MEN Mikro Elektronik GmbH, Nuremberg, Germany
 ****************************************************************************/

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <devctl.h>
#include <sys/dcmd_chr.h>
#include <MEN/men_typs.h>
#include <MEN/maccess.h>
#include <MEN/m77_drv.h>


/***********************************************************************/
/*
 * Display Program usage
 */
void usage(void)
{

    printf(" m77_devctl [hv]<-d device> [-t=0/1] [-p=0] [-s=0/1].\n");
    printf(" device: UART device\n");
    printf("\n");
    printf("Example for M45N specific devctls (Tristate control):\n");
    printf(" m77_devctl -d /dev/sern -t 1    set Chan. n to Tristate\n");
    printf(" m77_devctl -d /dev/sern -t 0    set Chan. n to normal Operation\n");
    printf("\n");

    printf("Example for M77 specific devctls (physical Mode Control):\n");
    printf(" m77_devctl -d /dev/sern -p 1    RS422 Halfduplex\n");
    printf(" m77_devctl -d /dev/sern -p 2    RS422 Fullduplex\n");
    printf(" m77_devctl -d /dev/sern -p 3    RS485 Halfduplex\n");
    printf(" m77_devctl -d /dev/sern -p 4    RS485 Fullduplex\n");
    printf(" m77_devctl -d /dev/sern -p 7    RS232\n");

    printf("Example for M77 specific devctls (Echo suppression in HD):\n");
    printf(" m77_devctl -d /dev/sern -s 0  suppress echo (DCR[RX_EN] = 0)\n");
    printf(" m77_devctl -d /dev/sern -s 1  Enable echo (DCR[RX_EN]   = 1)\n");
    printf("\n");

    printf(" Arguments without Value:\n");
    printf(" -v   verbose outputs of whats done\n");
    printf(" -h   help, dumps this usage text\n");
    exit(1);

}



/***********************************************************************/
/*
 * the only main function
 *
 */
int main(int argc, char *argv[])
{

    int fd        = 0;
    int option    = 0;
    int val       = 0;
    int retval    = 0;
    int nverbose  = 0;

    /* map given phy mode (equal to definition in serial_m77.h) to a string*/
    char *phyModes[8]={" ", "RS422HD", "RS422FD", "RS485HD", "RS485FD",
                       " ", " ", "RS232" };

    if (argc < 2)
        usage();

    while ((option = getopt(argc, argv, "vhd:t:p:s:")) >=0 ) {
        switch (option) {

        case 'v':
            if (nverbose == 0) {
                printf( "MEN M45N/M69N/M77 QNX driver devctl test\n");
                printf( " Build: %s %s\n\n", __DATE__, __TIME__);
                nverbose = 1;
            }
            break;

        case 'h':
            usage();
            break;

        case 'd':
            if (nverbose) {
                printf( "Opening UART device %s\n", optarg );
            }

            if ( (fd = open( optarg, O_RDWR)) == -1 ) {
                printf("*** cant open device %s !\n", optarg);
                exit(1);
            }
            break;

        case 'p':
            val = atoi(optarg);
            if ((val!=1) && (val!=2) && (val!=3) && (val!=4) && (val!=7))
            {
                printf("*** unknown PHY Mode '%d'. Use 1,2,3,4 or 7\n",val);
                exit(1);
            }

            if (nverbose) {
                printf("Set M77 Physical Mode to '%s'\n",  phyModes[val] );
            }
            retval = devctl (fd, M77_PHYS_INT_SET, &val, sizeof(val), NULL);

            break;

        case 't':
            val = atoi(optarg);
            if (nverbose) {
                printf("Set M45N Tristate Mode to %d\n", val ? 1 : 0 );
            }
            retval = devctl( fd, M45_TIO_TRI_MODE, &val, sizeof(val), NULL );
            break;

        case 's':
            val = atoi(optarg);
            if (nverbose) {
                printf("Set M77 Echo Mode to %d\n", val ? 1 : 0);
            }
            retval = devctl( fd, M77_ECHO_SUPPRESS, &val, sizeof(val), NULL );
            break;

        default:
            usage();
        }
    }

    if (retval) {
        printf("*** error %d while executing devctl!\n", retval);
    }

    close(fd);
    return retval;

};
