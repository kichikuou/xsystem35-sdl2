/*
 * music_ctlsocket.c  music server socket control funciton
 *
 * Copyright (C) 1997-1998 Masaki Chikama (Wren) <chikama@kasumi.ipl.mech.nagoya-u.ac.jp>
 *               1998-                           <masaki-c@is.aist-nara.ac.jp>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
*/
/* $Id: music_ctrlsocket.c,v 1.1 2002/08/18 09:35:29 chikama Exp $ */

#include <stdio.h>
#include <stdlib.h>

static void ctrl_write_packet(int fd, void* data, int length) {
	ServerPktHeader pkthdr;

	pkthdr.version = XSYS35_PROTOCOL_VERSION;
	pkthdr.data_length = length;
	write(fd, &pkthdr, sizeof (ServerPktHeader));
	if (data && length > 0) {
		write(fd, data, length);
	}
}

static void ctrl_write_int(int fd, int val) {
	ctrl_write_packet(fd, &val, sizeof (int));
}

static void ctrl_write_float(int fd, float val) {
	ctrl_write_packet(fd, &val, sizeof (float));
}

static void ctrl_write_boolean(int fd, boolean bool) {
	ctrl_write_packet(fd, &bool, sizeof (boolean));
}

static void ctrl_write_string(int fd, char* string) {
	ctrl_write_packet(fd, string, string ? strlen(string) + 1 : 0);
}

static void ctrl_ack_packet(PacketNode * pkt) {
        ctrl_write_packet(pkt->fd, NULL, 0);
	close(pkt->fd);
	if (pkt->data) {
		free(pkt->data);
	}
	free(pkt);
}
