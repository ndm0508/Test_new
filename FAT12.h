/*Thư viện chứa các biến, các hàm thao tác với file FAT12*/
#ifndef FAT12_H
#define FAT12_H

#include<stdio.h>
#include<stdint.h>
#include<string.h>
#define SECTOR_SIZE 512 // Định nghĩa kích thước của sector là 512 byte
#pragma pack(1) // Đảm bảo k có padding trong struct
//Khai báo biến boot sector
typedef struct {
    uint8_t jmp[3];
    char oem_name[8];
    uint16_t bytes_per_sector;
    uint8_t sectors_per_cluster;
    uint16_t reserved_sectors;
    uint8_t num_fats;
    uint16_t root_entries;
    uint16_t total_sectors_16;
    uint8_t media_descriptor;
    uint16_t sectors_per_fat;
    uint16_t sectors_per_track;
    uint16_t num_heads;
    uint32_t hidden_sectors;
    uint32_t total_sectors_32;
    uint8_t drive_number;
    uint8_t reserved;
    uint8_t signature;
    uint32_t volume_id;
    char volume_label[11];
    char file_system_type[8];
    char boot_code[448];
    uint16_t boot_signature;
} BootSector;
// Khai báo biến root directory
typedef struct {
    uint8_t name[8];
    uint8_t ext[3];
    uint8_t attributes;
    uint8_t reserved;
    uint8_t create_time_tenth;
    uint16_t create_time;
    uint16_t create_date;
    uint16_t last_access_date;
    uint16_t ignore;
    uint16_t last_modification_time;
    uint16_t last_modification_date;
    uint16_t first_cluster;
    uint32_t file_size;
} RootDirectory;
// Khai báo các hàm sử dụng
void print_boot_sector_info(const BootSector *boot_sector) ;
int32_t HAL_read_sector(uint32_t index, uint8_t *buff,char * path) ;
int32_t HAL_read_multi_sector(uint32_t index, uint32_t num, uint8_t *buff,char * path) ;
void print_directory_in_root(const BootSector *boot_sector,char * path) ;
void list_files_in_directory(uint16_t cluster,const BootSector *boot_sector,char * path) ;
void display_file_content(uint16_t cluster,const BootSector *boot_sector,char * path) ;

#endif // FAT12_H
