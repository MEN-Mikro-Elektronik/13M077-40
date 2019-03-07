/*********************  P r o g r a m  -  M o d u l e ***********************/
/*!
 *        \file  sys_ttyinit.c
 *
 *      \author  christophe.hannoyer@men.de
 *        $Date: 2009/06/24 16:24:30 $
 *    $Revision: 1.1 $
 *
 *       \brief  Initialize the tty structure
 *
 *               Initialize the tty structure that the driver passes
 *               to io-char.
 *
 *     Switches: -
 */
/*-------------------------------[ History ]---------------------------------
 *
 * $Log: sys_ttyinit.c,v $
 * Revision 1.1  2009/06/24 16:24:30  channoyer
 * Initial Revision
 *
 *---------------------------------------------------------------------------
 * (c) Copyright 2009 by MEN Mikro Elektronik GmbH, Nuremberg, Germany
 ****************************************************************************/

#include "externs.h"

#define DEFAULT_CLK 18432000
#define DEFAULT_DIV 16

void
sys_ttyinit( TTYINIT *dip ) {
    dip->clk = DEFAULT_CLK;
    dip->div = DEFAULT_DIV;
}
