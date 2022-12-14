#include "ntfs.h"
#include "main.h"
#include <stdio.h>
#include <time.h>

#define BUFSIZE 512
void traverseClusters(int fd, uint16_t cluster_size, uint16_t lba);

void findjpg(int fd) {
  uint8_t mbr_buffer[BUFSIZE];
  MBR* mbr;
  
  // first find start of partition. 
  read(fd, mbr_buffer, BUFSIZE);
  mbr = (MBR*) mbr_buffer;

  if (mbr->mbr_sig != 0xAA55) {
    printf("ERROR: Not in MBR\n");
    return;
  }

  // lseek to the start of partition. 
  PTE partition0 = mbr->mbr_ptable[0];
  uint8_t pbs_buffer[BUFSIZE];
  uint8_t vbr_buffer[73];
  uint16_t lba = partition0.pte_lba;
  PBS* pbs;
  VBR* vbr;
  
  lseek(fd, lba*512, SEEK_SET);
  read(fd, pbs_buffer, BUFSIZE);
  pbs = (PBS*) pbs_buffer;
  vbr = (VBR*) pbs->pbs_vbr;
  
  // find address of MFT
  // confirm the partition is NTFS (signature verification)
  if (pbs->pbs_sig != 0xAA55) {
    printf("ERROR: Not NTFS Partition\n");
    return;
  }
  
  // calculating cluster size
  uint16_t bps = vbr->vbr_bps; // bytes per sector
  uint8_t spc = vbr->vbr_spc; // sectors per cluster
  uint16_t cluster_size = bps * spc; // bytes per sector * sectors per cluster = bytes per cluster
 
  // scans partition one cluster at a time
  clock_t t;
  t = clock();

  traverseClusters(fd, cluster_size, lba*512);

  t = clock() - t;
  double time_taken = ((double)t)/CLOCKS_PER_SEC; // in seconds 
  printf("\nTime elapsed: %f\n", time_taken);
  
  return;
}


void traverseClusters(int fd, uint16_t cluster_size, uint16_t lba) {

  uint8_t cluster_buffer[cluster_size];
  uint64_t cluster_loc;
  int flag = 0;
  int fsize[2];
  
  // traverse each cluster
  for(int i=0; i<250000; i++) {
    cluster_loc = lba + (cluster_size * i);

    // zero out cluster_buffer
    for(int a=0; a<cluster_size; a++)
      cluster_buffer[a] = 0;

    // read 0x1000 (cluster) into buffer
    lseek(fd, cluster_loc, SEEK_SET);
    read(fd, cluster_buffer, cluster_size);

    // traverses each bit to find jpg
    for(int j=0; j<(cluster_size-4); j++)
      if(cluster_buffer[j] == 0xFF)
	if(cluster_buffer[j+1] == 0xD8 && cluster_buffer[j+2] == 0xFF && cluster_buffer[j+3] == 0xE0 && flag == 0) {
	  printf("\njpg BOF: j=%d\t@ 0x%08lX\tCluster: %d\n", j, cluster_loc + j, i);
	  flag = 1;
	  fsize[0] = cluster_loc + j;
	  break;
	}
	else if(cluster_buffer[j+1] == 0xD9 && flag == 1) {
	  fsize[1] = cluster_loc + j;
	  printf("jpg EOF: j=%d\t@ 0x%08lX\tFile Size: %d bytes\n", j, cluster_loc + j, fsize[1] - fsize[0]);
	  flag = 0;
	  break;
	}
  }
}
