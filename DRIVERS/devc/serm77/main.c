/*********************  P r o g r a m  -  M o d u l e ***********************/
/*!
 *        \file  main.c
 *
 *      \author  christophe.hannoyer@men.de
 *        $Date: 2009/06/24 16:23:18 $
 *    $Revision: 1.1 $
 *
 *       \brief  The main part of the driver
 *
 *     Switches: -
 */
/*-------------------------------[ History ]---------------------------------
 *
 * $Log: main.c,v $
 * Revision 1.1  2009/06/24 16:23:18  channoyer
 * Initial Revision
 *
 *---------------------------------------------------------------------------
 * (c) Copyright 2009 by MEN Mikro Elektronik GmbH, Nuremberg, Germany
 ****************************************************************************/

#include "externs.h"

int
main(int argc, char *argv[]) {

    ttyctrl.max_devs = 16;
    ttc(TTC_INIT_PROC, &ttyctrl, 24);

    if(options(argc, argv) == 0) {
        fprintf(stderr, "%s: No serial ports found\n", argv[0]);
        exit(0);
    }

    ttc(TTC_INIT_START, &ttyctrl, 0);
    return 0;
}
