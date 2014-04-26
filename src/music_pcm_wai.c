#include <string.h>
#include <glib.h>
#include <errno.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "portab.h"
#include "system.h"
#include "nact.h"
#include "LittleEndian.h"

static char *wai_mapadr;

// Volume mixer channel
#define WAIMIXCH(no) LittleEndian_getDW(wai_mapadr, 36 + (no -1) * 4 * 3 + 8)

// WAVFILE マーカー?
#define WAIMARK(no)  LittleEndian_getDW(wai_mapadr, 36 + (no -1) * 4 * 3 + 4)


int muspcm_load_wai() {
	struct stat sbuf;
	char *adr;
	int fd;

	wai_mapadr = NULL;
	if (nact->files.wai == NULL) goto endwai;
	
	if (0 > (fd = open(nact->files.wai, O_RDONLY))) {
		WARNING("open: %s\n", strerror(errno));
		goto endwai;
	}
	
	if (0 > fstat(fd, &sbuf)) {
		WARNING("fstat:%s\n", strerror(errno));
		close(fd);
		goto endwai;
	}
	
	if (MAP_FAILED == (adr = mmap(0, sbuf.st_size, PROT_READ, MAP_SHARED, fd, 0))) {
		WARNING("mmap: %s\n", strerror(errno));
		close(fd);
		goto endwai;
	}
	
	if (*adr != 'X' || *(adr+1) != 'I' || *(adr+2) != '2') {
		WARNING("not WAI file\n");
		munmap(adr, sbuf.st_size);
		close(fd);
		goto endwai;
	}
	
	wai_mapadr = adr;
	
 endwai:
	return OK;
}

