/******************************************************************************
 *
 * Copyright(c) 2009-2014  Realtek Corporation.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * The full GNU General Public License is included in this distribution in the
 * file called LICENSE.
 *
 * Contact Information:
 * wlanfae <wlanfae@realtek.com>
 * Realtek Corporation, No. 2, Innovation Road II, Hsinchu Science Park,
 * Hsinchu 300, Taiwan.
 *
 * Larry Finger <Larry.Finger@lwfinger.net>
 *
 *****************************************************************************/

#ifndef __RTL8723BE_SW_H__
#define __RTL8723BE_SW_H__

int rtl8723be_init_sw_vars(struct ieee80211_hw *hw);
void rtl8723be_deinit_sw_vars(struct ieee80211_hw *hw);
void rtl8723be_init_var_map(struct ieee80211_hw *hw);
bool rtl8723be_get_btc_status(void);

#endif
