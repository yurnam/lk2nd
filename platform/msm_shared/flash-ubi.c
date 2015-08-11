/* Copyright (c) 2015, The Linux Foundation. All rights reserved.

 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above
 *     copyright notice, this list of conditions and the following
 *     disclaimer in the documentation and/or other materials provided
 *     with the distribution.
 *   * Neither the name of The Linux Foundation. nor the names of its
 *     contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
 * IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <stdlib.h>
#include <string.h>
#include <dev/flash-ubi.h>
#include <dev/flash.h>
#include <qpic_nand.h>
#include <rand.h>

static
const uint32_t crc32_table[256] = {
	0x00000000L, 0x77073096L, 0xee0e612cL, 0x990951baL, 0x076dc419L,
	0x706af48fL, 0xe963a535L, 0x9e6495a3L, 0x0edb8832L, 0x79dcb8a4L,
	0xe0d5e91eL, 0x97d2d988L, 0x09b64c2bL, 0x7eb17cbdL, 0xe7b82d07L,
	0x90bf1d91L, 0x1db71064L, 0x6ab020f2L, 0xf3b97148L, 0x84be41deL,
	0x1adad47dL, 0x6ddde4ebL, 0xf4d4b551L, 0x83d385c7L, 0x136c9856L,
	0x646ba8c0L, 0xfd62f97aL, 0x8a65c9ecL, 0x14015c4fL, 0x63066cd9L,
	0xfa0f3d63L, 0x8d080df5L, 0x3b6e20c8L, 0x4c69105eL, 0xd56041e4L,
	0xa2677172L, 0x3c03e4d1L, 0x4b04d447L, 0xd20d85fdL, 0xa50ab56bL,
	0x35b5a8faL, 0x42b2986cL, 0xdbbbc9d6L, 0xacbcf940L, 0x32d86ce3L,
	0x45df5c75L, 0xdcd60dcfL, 0xabd13d59L, 0x26d930acL, 0x51de003aL,
	0xc8d75180L, 0xbfd06116L, 0x21b4f4b5L, 0x56b3c423L, 0xcfba9599L,
	0xb8bda50fL, 0x2802b89eL, 0x5f058808L, 0xc60cd9b2L, 0xb10be924L,
	0x2f6f7c87L, 0x58684c11L, 0xc1611dabL, 0xb6662d3dL, 0x76dc4190L,
	0x01db7106L, 0x98d220bcL, 0xefd5102aL, 0x71b18589L, 0x06b6b51fL,
	0x9fbfe4a5L, 0xe8b8d433L, 0x7807c9a2L, 0x0f00f934L, 0x9609a88eL,
	0xe10e9818L, 0x7f6a0dbbL, 0x086d3d2dL, 0x91646c97L, 0xe6635c01L,
	0x6b6b51f4L, 0x1c6c6162L, 0x856530d8L, 0xf262004eL, 0x6c0695edL,
	0x1b01a57bL, 0x8208f4c1L, 0xf50fc457L, 0x65b0d9c6L, 0x12b7e950L,
	0x8bbeb8eaL, 0xfcb9887cL, 0x62dd1ddfL, 0x15da2d49L, 0x8cd37cf3L,
	0xfbd44c65L, 0x4db26158L, 0x3ab551ceL, 0xa3bc0074L, 0xd4bb30e2L,
	0x4adfa541L, 0x3dd895d7L, 0xa4d1c46dL, 0xd3d6f4fbL, 0x4369e96aL,
	0x346ed9fcL, 0xad678846L, 0xda60b8d0L, 0x44042d73L, 0x33031de5L,
	0xaa0a4c5fL, 0xdd0d7cc9L, 0x5005713cL, 0x270241aaL, 0xbe0b1010L,
	0xc90c2086L, 0x5768b525L, 0x206f85b3L, 0xb966d409L, 0xce61e49fL,
	0x5edef90eL, 0x29d9c998L, 0xb0d09822L, 0xc7d7a8b4L, 0x59b33d17L,
	0x2eb40d81L, 0xb7bd5c3bL, 0xc0ba6cadL, 0xedb88320L, 0x9abfb3b6L,
	0x03b6e20cL, 0x74b1d29aL, 0xead54739L, 0x9dd277afL, 0x04db2615L,
	0x73dc1683L, 0xe3630b12L, 0x94643b84L, 0x0d6d6a3eL, 0x7a6a5aa8L,
	0xe40ecf0bL, 0x9309ff9dL, 0x0a00ae27L, 0x7d079eb1L, 0xf00f9344L,
	0x8708a3d2L, 0x1e01f268L, 0x6906c2feL, 0xf762575dL, 0x806567cbL,
	0x196c3671L, 0x6e6b06e7L, 0xfed41b76L, 0x89d32be0L, 0x10da7a5aL,
	0x67dd4accL, 0xf9b9df6fL, 0x8ebeeff9L, 0x17b7be43L, 0x60b08ed5L,
	0xd6d6a3e8L, 0xa1d1937eL, 0x38d8c2c4L, 0x4fdff252L, 0xd1bb67f1L,
	0xa6bc5767L, 0x3fb506ddL, 0x48b2364bL, 0xd80d2bdaL, 0xaf0a1b4cL,
	0x36034af6L, 0x41047a60L, 0xdf60efc3L, 0xa867df55L, 0x316e8eefL,
	0x4669be79L, 0xcb61b38cL, 0xbc66831aL, 0x256fd2a0L, 0x5268e236L,
	0xcc0c7795L, 0xbb0b4703L, 0x220216b9L, 0x5505262fL, 0xc5ba3bbeL,
	0xb2bd0b28L, 0x2bb45a92L, 0x5cb36a04L, 0xc2d7ffa7L, 0xb5d0cf31L,
	0x2cd99e8bL, 0x5bdeae1dL, 0x9b64c2b0L, 0xec63f226L, 0x756aa39cL,
	0x026d930aL, 0x9c0906a9L, 0xeb0e363fL, 0x72076785L, 0x05005713L,
	0x95bf4a82L, 0xe2b87a14L, 0x7bb12baeL, 0x0cb61b38L, 0x92d28e9bL,
	0xe5d5be0dL, 0x7cdcefb7L, 0x0bdbdf21L, 0x86d3d2d4L, 0xf1d4e242L,
	0x68ddb3f8L, 0x1fda836eL, 0x81be16cdL, 0xf6b9265bL, 0x6fb077e1L,
	0x18b74777L, 0x88085ae6L, 0xff0f6a70L, 0x66063bcaL, 0x11010b5cL,
	0x8f659effL, 0xf862ae69L, 0x616bffd3L, 0x166ccf45L, 0xa00ae278L,
	0xd70dd2eeL, 0x4e048354L, 0x3903b3c2L, 0xa7672661L, 0xd06016f7L,
	0x4969474dL, 0x3e6e77dbL, 0xaed16a4aL, 0xd9d65adcL, 0x40df0b66L,
	0x37d83bf0L, 0xa9bcae53L, 0xdebb9ec5L, 0x47b2cf7fL, 0x30b5ffe9L,
	0xbdbdf21cL, 0xcabac28aL, 0x53b39330L, 0x24b4a3a6L, 0xbad03605L,
	0xcdd70693L, 0x54de5729L, 0x23d967bfL, 0xb3667a2eL, 0xc4614ab8L,
	0x5d681b02L, 0x2a6f2b94L, 0xb40bbe37L, 0xc30c8ea1L, 0x5a05df1bL,
	0x2d02ef8dL
};

static uint32_t mtd_crc32(uint32_t crc, const void *buf, size_t size)
{
	const uint8_t *p = buf;

	while (size--)
		crc = crc32_table[(crc ^ *p++) & 0xff] ^ (crc >> 8);
	return crc;
}

/**
 * check_pattern - check if buffer contains only a certain byte pattern.
 * @buf: buffer to check
 * @patt: the pattern to check
 * @size: buffer size in bytes
 *
 * This function returns %1 if there are only @patt bytes in @buf, and %0 if
 * something else was also found.
 */
int check_pattern(const void *buf, uint8_t patt, int size)
{
	int i;

	for (i = 0; i < size; i++)
		if (((const uint8_t *)buf)[i] != patt)
			return 0;
	return 1;
}

/**
 * read_ec_hdr - read and check an erase counter header.
 * @peb: number of the physical erase block to read the header for
 * @ec_hdr: a &struct ubi_ec_hdr object where to store the read erase counter
 * 			header
 *
 * This function reads erase counter header from physical eraseblock @peb and
 * stores it in @ec_hdr. This function also checks the validity of the read
 * header.
 *
 * Return codes:
 * -1 - in case of error
 *  0 - if PEB was found valid
 *  1 - if PEB is empty
 */
static int read_ec_hdr(uint32_t peb, struct ubi_ec_hdr *ec_hdr)
{
	unsigned char *spare, *tmp_buf;
	int ret = -1;
	uint32_t crc;
	int page_size = flash_page_size();
	int num_pages_per_blk = flash_block_size()/page_size;

	spare = (unsigned char *)malloc(flash_spare_size());
	if (!spare)
	{
		dprintf(CRITICAL, "read_ec_hdr: Mem allocation failed\n");
		return ret;
	}

	tmp_buf = (unsigned char *)malloc(page_size);
	if (!tmp_buf)
	{
		dprintf(CRITICAL, "read_ec_hdr: Mem allocation failed\n");
		goto out_tmp_buf;
	}

	if (qpic_nand_block_isbad(peb * num_pages_per_blk)) {
		dprintf(CRITICAL, "read_ec_hdr: Bad block @ %d\n", peb);
		goto out;
	}

	if (qpic_nand_read(peb * num_pages_per_blk, 1, tmp_buf, spare)) {
		dprintf(CRITICAL, "read_ec_hdr: Read %d failed \n", peb);
		goto out;
	}
	memcpy(ec_hdr, tmp_buf, UBI_EC_HDR_SIZE);

	if (check_pattern((void *)ec_hdr, 0xFF, UBI_EC_HDR_SIZE)) {
		ret = 1;
		goto out;
	}

	/* Make sure we read a valid UBI EC_HEADER */
	if (BE32(ec_hdr->magic) != (uint32_t)UBI_EC_HDR_MAGIC) {
		dprintf(CRITICAL,
			"read_ec_hdr: Wrong magic at peb-%d Expected: %d, received %d\n",
			peb, UBI_EC_HDR_MAGIC, BE32(ec_hdr->magic));
		goto out;
	}

	if (ec_hdr->version != UBI_VERSION) {
		dprintf(CRITICAL,
			"read_ec_hdr: Wrong version at peb-%d Expected: %d, received %d\n",
			peb, UBI_VERSION, ec_hdr->version);
		goto out;
	}

	if (BE64(ec_hdr->ec) > UBI_MAX_ERASECOUNTER) {
		dprintf(CRITICAL,
			"read_ec_hdr: Wrong ec at peb-%d: %lld \n",
			peb, BE64(ec_hdr->ec));
		goto out;
	}

	crc = mtd_crc32(UBI_CRC32_INIT, ec_hdr, UBI_EC_HDR_SIZE_CRC);
	if (BE32(ec_hdr->hdr_crc) != crc) {
		dprintf(CRITICAL,
			"read_ec_hdr: Wrong crc at peb-%d: calculated %d, recived %d\n",
			peb,crc,  BE32(ec_hdr->hdr_crc));
		goto out;
	}

	ret = 0;
out:
	free(tmp_buf);
out_tmp_buf:
	free(spare);
	return ret;
}

/**
 * write_ec_header() - Write provided ec_header for given PEB
 * @peb: number of the physical erase block to write the header to
 * @new_ech: the ec_header to write
 *
 * Return codes:
 * -1 - in case of error
 *  0 - on success
 */
static int write_ec_header(uint32_t peb, struct ubi_ec_hdr *new_ech)
{
	unsigned page_size = flash_page_size();
	int num_pages_per_blk = flash_block_size()/page_size;
	unsigned char *buf;
	int ret = 0;

	buf = malloc(sizeof(uint8_t) * page_size);
	if (!buf) {
		dprintf(CRITICAL, "write_ec_header: Mem allocation failed\n");
		return -1;
	}

	memset(buf, 0, page_size);
	ASSERT(page_size > sizeof(*new_ech));
	memcpy(buf, new_ech, UBI_EC_HDR_SIZE);
	ret = qpic_nand_write(peb * num_pages_per_blk, 1, buf, 0);
	if (ret) {
		dprintf(CRITICAL,
			"write_ec_header: qpic_nand_write failed with %d\n", ret);
		ret = -1;
		goto out;
	}

out:
	free(buf);
	return ret;
}

/**
 * scan_partition() - Collect the ec_headers info of a given partition
 * @ptn: partition to read the headers of
 *
 * Returns allocated and filled struct ubi_scan_info (si).
 * Note: si should be released by caller.
 */
static struct ubi_scan_info *scan_partition(struct ptentry *ptn)
{
	struct ubi_scan_info *si;
	struct ubi_ec_hdr *ec_hdr;
	unsigned i, curr_peb;
	unsigned long long sum = 0;
	int page_size = flash_page_size();
	int ret;

	si = malloc(sizeof(*si));
	if (!si) {
		dprintf(CRITICAL,"scan_partition: (%s) Memory allocation failed\n",
				ptn->name);
		return NULL;
	}

	memset((void *)si, 0, sizeof(*si));
	si->ec = malloc(ptn->length * sizeof(uint64_t));
	if (!si->ec) {
		dprintf(CRITICAL,"scan_partition: (%s) Memory allocation failed\n",
				ptn->name);
		goto out_failed_ec;
	}
	memset((void *)si->ec, 0, ptn->length * sizeof(uint64_t));

	ec_hdr = malloc(UBI_EC_HDR_SIZE);
	if (!ec_hdr) {
		dprintf(CRITICAL,"scan_partition: (%s) Memory allocation failed\n",
				ptn->name);
		goto out_failed;
	}

	curr_peb = ptn->start;
	si->vid_hdr_offs = 0;
	si->image_seq = rand() & UBI_IMAGE_SEQ_BASE;

	for (i = 0; i < ptn->length; i++){
		ret = read_ec_hdr(curr_peb + i, ec_hdr);
		switch (ret) {
		case 1:
			si->empty_cnt++;
			si->ec[i] = UBI_MAX_ERASECOUNTER;
			break;
		case 0:
			if (!si->vid_hdr_offs) {
				si->vid_hdr_offs = BE32(ec_hdr->vid_hdr_offset);
				si->data_offs = BE32(ec_hdr->data_offset);
				if (!si->vid_hdr_offs || !si->data_offs ||
					si->vid_hdr_offs % page_size ||
					si->data_offs % page_size) {
					si->bad_cnt++;
					si->ec[i] = UBI_MAX_ERASECOUNTER;
					si->vid_hdr_offs = 0;
					continue;
				}
				if (BE32(ec_hdr->vid_hdr_offset) != si->vid_hdr_offs) {
					si->bad_cnt++;
					si->ec[i] = UBI_MAX_ERASECOUNTER;
					continue;
				}
				if (BE32(ec_hdr->data_offset) != si->data_offs) {
					si->bad_cnt++;
					si->ec[i] = UBI_MAX_ERASECOUNTER;
					continue;
				}
			}
			si->good_cnt++;
			si->ec[i] = BE64(ec_hdr->ec);
			break;
		case -1:
		default:
			si->bad_cnt++;
			si->ec[i] = UBI_MAX_ERASECOUNTER;
			break;
		}
	}

	/*
	 * If less then 95% of the PEBs were "bad" (didn't have valid
	 * ec header), then set mean_ec = UBI_DEF_ERACE_COUNTER.
	 */
	sum = 0;
	if (si->good_cnt && (double)(si->good_cnt / ptn->length) * 100 > 95) {
		for (i = 0; i < ptn->length; i++) {
			if (si->ec[i] == UBI_MAX_ERASECOUNTER)
				continue;
			sum += si->ec[i];
		}
		si->mean_ec = sum / si->good_cnt;
	} else {
		si->mean_ec = UBI_DEF_ERACE_COUNTER;
	}
	free(ec_hdr);
	return si;

out_failed:
	free(si->ec);
out_failed_ec:
	free(si);
	return NULL;
}

/**
 * update_ec_header() - Update provided ec_header
 * @si: pointer to struct ubi_scan_info, holding the collected
 * 		ec_headers information of the partition
 * @index: index in si->ec[] of this peb. Relative to ptn->start
 * @new_header: False if this is an update of an existing header.
 * 		True if new header needs to be filled in
 */
static void update_ec_header(struct ubi_ec_hdr *old_ech,
		const struct ubi_scan_info *si,
		int index, bool new_header)
{
	uint32_t crc;

	if (si->ec[index] < UBI_MAX_ERASECOUNTER)
		old_ech->ec = BE64(si->ec[index] + 1);
	else
		old_ech->ec = BE64(si->mean_ec);

	if (new_header) {
		old_ech->vid_hdr_offset = BE32(si->vid_hdr_offs);
		old_ech->data_offset = BE32(si->data_offs);
		old_ech->magic = BE32(UBI_EC_HDR_MAGIC);
		old_ech->version = UBI_VERSION;
	}
	old_ech->image_seq = BE32(si->image_seq);
	crc = mtd_crc32(UBI_CRC32_INIT,
			(const void *)old_ech, UBI_EC_HDR_SIZE_CRC);
	old_ech->hdr_crc = BE32(crc);
}

/**
 * calc_data_len - calculate how much real data is stored in the buffer
 * @page_size: min I/O of the device
 * @buf: a buffer with the contents of the physical eraseblock
 * @len: the buffer length
 *
 * This function calculates how much "real data" is stored in @buf and
 * returns the length (in number of pages). Continuous 0xFF bytes at the end
 * of the buffer are not considered as "real data".
 */
static int calc_data_len(int page_size, const void *buf, int len)
{
	int i;

	for (i = len - 1; i >= 0; i--)
		if (((const uint8_t *)buf)[i] != 0xFF)
			break;

	/* The resulting length must be aligned to the minimum flash I/O size */
	len = i + 1;
	len = (len + page_size - 1) / page_size;
	return len;
}

/**
 * flash_ubi_img() - Write the provided (UBI) image to given partition
 * @ptn: partition to write the image to
 * @data: the image to write
 * @size: size of the image to write
 *
 *  Return codes:
 * -1 - in case of error
 *  0 - on success
 */
int flash_ubi_img(struct ptentry *ptn, void *data, unsigned size)
{
	struct ubi_scan_info *si;
	struct ubi_ec_hdr *old_ech;
	struct ubi_ec_hdr new_ech;
	uint32_t curr_peb = ptn->start;
	void *img_peb;
	unsigned page_size = flash_page_size();
	unsigned block_size = flash_block_size();
	int num_pages_per_blk = block_size / page_size;
	int num_pages;
	int ret, need_erase;

	si = scan_partition(ptn);
	if (!si) {
		dprintf(CRITICAL, "flash_ubi_img: scan_partition failed\n");
		return -1;
	}

	/*
	 * In case si->vid_hdr_offs is still -1 (non UBI image was
	 * flashed on device, get the value from the image to flush
	 */
	if (!si->vid_hdr_offs){
		struct ubi_ec_hdr *echd = (struct ubi_ec_hdr *)data;
		si->vid_hdr_offs = BE32(echd->vid_hdr_offset);
		si->data_offs = BE32(echd->data_offset);
	}

	need_erase = (si->empty_cnt == (int)ptn->length ? 0 : 1);
	/* Update the "to be" flashed image and flash it */
	img_peb = data;
	while (size && curr_peb < ptn->start + ptn->length) {
		if (need_erase && qpic_nand_blk_erase(curr_peb * num_pages_per_blk)) {
			dprintf(CRITICAL, "flash_ubi_img: erase of %d failed\n",
				curr_peb);
			curr_peb++;
			continue;
		}

		/* Update the ec_header in the image */
		old_ech = (struct ubi_ec_hdr *)img_peb;
		update_ec_header(old_ech, si, curr_peb - ptn->start, false);
		if (size < block_size)
			num_pages = size / page_size;
		else
			num_pages = calc_data_len(page_size, img_peb, block_size);
		/* Write one block from image */
		ret = qpic_nand_write(curr_peb * num_pages_per_blk,
				num_pages, img_peb, 0);
		if (ret) {
			dprintf(CRITICAL, "flash_ubi_img: writing to peb-%d failed\n",
					curr_peb);
			curr_peb++;
			continue;
		}
		if (size < block_size)
			size = 0;
		else
			size -= block_size;
		img_peb += flash_block_size();
		curr_peb++;
	}

	if (size) {
		dprintf(CRITICAL,
				"flash_ubi_img: Not enough good blocks to flash image!");
		ret = -1;
		goto out;
	}

	/* Erase and write ec_header for the rest of the blocks */
	for (; curr_peb < ptn->start + ptn->length; curr_peb++) {
		if (need_erase && qpic_nand_blk_erase(curr_peb * num_pages_per_blk)) {
			dprintf(INFO, "flash_ubi_img: erase of %d failed\n", curr_peb);
			curr_peb++;
			continue;
		}
		memset(&new_ech, 0xff, sizeof(new_ech));
		update_ec_header(&new_ech, si, curr_peb - ptn->start, true);

		/* Write new ec_header */
		ret = write_ec_header(curr_peb, &new_ech);
		if (ret) {
			dprintf(CRITICAL, "flash_ubi_img: write ec_header to %d failed\n",
					curr_peb);
			curr_peb++;
			continue;
		}
	}
	ret = 0;
out:
	free(si->ec);
	free(si);
	return ret;
}
