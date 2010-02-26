/*
 *  Networking under POSIX
 *  Copyright (C) 2007-2008 Andreas Öman
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdlib.h>
#include "net.h"

/**
 *
 */
int
tcp_write_queue(int fd, htsbuf_queue_t *q)
{
  htsbuf_data_t *hd;
  int l, r = 0;

  while((hd = TAILQ_FIRST(&q->hq_q)) != NULL) {
    TAILQ_REMOVE(&q->hq_q, hd, hd_link);

    l = hd->hd_data_len - hd->hd_data_off;
    r |= tcp_write(fd, hd->hd_data + hd->hd_data_off, l);
    free(hd->hd_data);
    free(hd);
  }
  q->hq_size = 0;
  return 0;
}


/**
 *
 */
int
tcp_write_queue_dontfree(int fd, htsbuf_queue_t *q)
{
  htsbuf_data_t *hd;
  int l, r = 0;

  TAILQ_FOREACH(hd, &q->hq_q, hd_link) {
    l = hd->hd_data_len - hd->hd_data_off;
    r |= tcp_write(fd, hd->hd_data + hd->hd_data_off, l);
  }
  return 0;
}


/**
 *
 */
static int
tcp_fill_htsbuf_from_fd(int fd, htsbuf_queue_t *hq)
{
  htsbuf_data_t *hd = TAILQ_LAST(&hq->hq_q, htsbuf_data_queue);
  int c;

  if(hd != NULL) {
    /* Fill out any previous buffer */
    c = hd->hd_data_size - hd->hd_data_len;

    if(c > 0) {

      if((c = tcp_read(fd, hd->hd_data + hd->hd_data_len, c, 0)) < 0)
	return -1;

      hd->hd_data_len += c;
      hq->hq_size += c;
      return 0;
    }
  }
  
  hd = malloc(sizeof(htsbuf_data_t));
  
  hd->hd_data_size = 1000;
  hd->hd_data = malloc(hd->hd_data_size);

  if((c = tcp_read(fd, hd->hd_data, hd->hd_data_size, 0)) < 0) {
    free(hd->hd_data);
    free(hd);
    return -1;
  }
  hd->hd_data_len = c;
  hd->hd_data_off = 0;
  TAILQ_INSERT_TAIL(&hq->hq_q, hd, hd_link);
  hq->hq_size += c;
  return 0;
}


/**
 *
 */
int
tcp_read_line(int fd, char *buf, const size_t bufsize, htsbuf_queue_t *spill)
{
  int len;

  while(1) {
    len = htsbuf_find(spill, 0xa);

    if(len == -1) {
      if(tcp_fill_htsbuf_from_fd(fd, spill) < 0)
	return -1;
      continue;
    }
    
    if(len >= bufsize - 1)
      return -1;

    htsbuf_read(spill, buf, len);
    buf[len] = 0;
    while(len > 0 && buf[len - 1] < 32)
      buf[--len] = 0;
    htsbuf_drop(spill, 1); /* Drop the \n */
    return 0;
  }
}


/**
 *
 */
int
tcp_read_data(int fd, char *buf, const size_t bufsize, htsbuf_queue_t *spill)
{
  int tot = htsbuf_read(spill, buf, bufsize);

  if(tot == bufsize)
    return 0;

  return tcp_read(fd, buf + tot, bufsize - tot, 1) < 0 ? -1 : 0;
}

/**
 *
 */
int
tcp_read_data2(int fd, char *buf, const size_t bufsize, htsbuf_queue_t *spill)
{
  int tot = htsbuf_read(spill, buf, bufsize);

  if(tot > 0)
    return tot;

  return tcp_read(fd, buf + tot, bufsize - tot, 0);
}
