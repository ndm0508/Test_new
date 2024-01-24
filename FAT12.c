/*Thư viện các hàm để thao tác với File FAT12*/

#include<stdint.h>
#include<string.h>
#include"FAT12.h"

// Hàm in thông tin của boot sector
void print_boot_sector_info(const BootSector *boot_sector)
{
    printf("Boot Sector Information:\n");
    printf("Jump code: %X %X %X\n",boot_sector->jmp[0],boot_sector->jmp[1], boot_sector->jmp[2]);
    printf("OEM Name: %.8s\n", boot_sector->oem_name);
    printf("Bytes Per Sector: %u\n", boot_sector->bytes_per_sector);
    printf("Sectors Per Cluster: %u\n", boot_sector->sectors_per_cluster);
    printf("Reserved Sectors: %u\n", boot_sector->reserved_sectors);
    printf("Number of FATs: %u\n", boot_sector->num_fats);
    printf("Root Entries: %u\n", boot_sector->root_entries);
    printf("Total Sectors (16-bit): %u\n", boot_sector->total_sectors_16);
    printf("Media Descriptor: 0x%02X\n", boot_sector->media_descriptor);
    printf("Sectors Per FAT: %u\n", boot_sector->sectors_per_fat);
    printf("Sectors Per Track: %u\n", boot_sector->sectors_per_track);
    printf("Number of Heads: %u\n", boot_sector->num_heads);
    printf("Hidden Sectors: %u\n", boot_sector->hidden_sectors);
    printf("Total Sectors (32-bit): %u\n", boot_sector->total_sectors_32);
    printf("Drive Number: 0x%02X\n", boot_sector->drive_number);
    printf("Signature: 0x%02X\n", boot_sector->signature);
    printf("Volume ID: 0x%08X\n", boot_sector->volume_id);
    printf("Volume Label: %.11s\n", boot_sector->volume_label);
    printf("File System Type: %.8s\n", boot_sector->file_system_type);
}


int32_t HAL_read_sector(uint32_t index, uint8_t *buff,char * path)
{
    FILE *imageFile = fopen(path, "rb"); // đọc file FAT12
    if (imageFile == NULL) {
        return 0; // Lỗi không thể mở file kết thúc chương trình
    }

    //Tìm vị trí bắt đầu của sector để đọc
    fseek(imageFile, index * SECTOR_SIZE, SEEK_SET);

    // Đọc sector ghi vào buffer
    size_t bytesRead = fread(buff, 1, SECTOR_SIZE, imageFile);

    fclose(imageFile);

    return bytesRead; // Trả về số bytes đọc được của sector
}

int32_t HAL_read_multi_sector(uint32_t index, uint32_t num, uint8_t *buff,char * path)
{
    int32_t totalBytesRead = 0;
    for (uint32_t i = 0; i < num; i++) {
    int32_t bytesRead = HAL_read_sector(index + i, buff + (i * SECTOR_SIZE),path);
    if (bytesRead < 0)
    {
            return 0; // Thoát không đọc được sector
    }
        totalBytesRead += bytesRead;
    }

    return totalBytesRead;// Trả về tổng số bytes đọc được từ vị trí index
}
// Hàm in các thông tin trong root directory
void print_directory_in_root(const BootSector *boot_sector,char * path)
{
    // Tính toán vị trí cluster bắt đầu vùng root
    uint32_t current_root = boot_sector->reserved_sectors + boot_sector->num_fats * boot_sector->sectors_per_fat;
    // Tinh toán số lượng sector của vùng root
    uint32_t root_sector =  boot_sector->root_entries *32/512;
    printf("current_root: %d\n", current_root);
    printf("root_sector: %d\n", root_sector);
    // Đọc thông tin của root directory
    uint8_t sector[SECTOR_SIZE];
    RootDirectory entry;
    for (int i = current_root; i < (current_root+root_sector); i++)
    {
        int32_t bytesRead = HAL_read_sector(i, sector,path);
        for (int m = 0; m < 16; m++)
        {
            memcpy((uint8_t *)&entry, &sector[m * 32], 32);
            // Kiểm tra đặc tính của entry name
            if (entry.name[0] == 0x00)
                {
                    return;
                }
            if (entry.name[0] == 0xE5)
            {
                continue;
            }
            // Không check long file name
            // Kiểm tra thuộc tính
            if (entry.attributes == 0x00)//Không có thuộc tính là file bình thường
            {
                printf("File Name: %.8s.", entry.name);
                printf("%.3s         ", entry.ext);
                printf("Position cluster: %d\n", entry.first_cluster);
            }
            if (entry.attributes == 0x10)//Là thư mục con
            {
                printf("Subdirectory name: %.8s     ", entry.name);
                printf("Position cluster: %d\n", entry.first_cluster);
            }

       }
    }
}
// Hàm liệt kê các file trong sub directory
void list_files_in_directory(uint16_t cluster,const BootSector *boot_sector,char * path)
{
    uint16_t element[512*(boot_sector->sectors_per_fat)*2/3];
    uint8_t FAT12 [(boot_sector->sectors_per_fat)*512];
    int32_t bytesRead = HAL_read_multi_sector(boot_sector->reserved_sectors,boot_sector->sectors_per_fat, FAT12,path);
    int num=0;
    // Chuyển đổi từ mảng uint8_t thành mảng uint16_t
    for(int m = 0; m < (512*(boot_sector->sectors_per_fat)/3); m++)
    {
        for (int n = 2*m; n< (2*m+2); n++)
        {
            num++;
            if(num==1)
            {
                element[n] = (uint16_t)(FAT12[n+m])|(uint16_t)((FAT12[n+m+1]&0x0F)<<8);
            }
            else if(num==2)
            {
                num=0;
                element[n] = (uint16_t)(FAT12[n+m]>>4)|(uint16_t)(FAT12[n+m+1]<<4);
            }
        }
    }
    //Thực hiện tương tự hàm print_root
    uint32_t current_root = boot_sector->reserved_sectors + boot_sector->num_fats * boot_sector->sectors_per_fat;
    uint32_t root_sector =  boot_sector->root_entries *32/512;
    uint8_t sectors[SECTOR_SIZE*(boot_sector->sectors_per_cluster)];
    RootDirectory entry;
    int count=0;int posi_clus[10000];
    posi_clus[count]=cluster;
    // Đếm số phần tử cần duyệt
    while (element[cluster] != 0x0FFF)
    {
        count ++;
        posi_clus[count]= element[cluster];
        cluster =element[cluster];
    }
    for (int i = 0; i <= count; i++)
    {
        bytesRead = HAL_read_multi_sector((current_root+root_sector+(posi_clus[i]-2)*(boot_sector->sectors_per_cluster)),boot_sector->sectors_per_cluster,sectors,path);
        for (int m = 0; m < (boot_sector->sectors_per_cluster)*16; m++)
        {
            memcpy((uint8_t *)&entry, &sectors[m * 32], 32);
            // Kiểm tra  entry name
            if (entry.name[0] == 0x00)
                return;
            if (entry.name[0] == 0xE5)
                continue;
            // Bỏ qua kiểm tra long file name
            // Kiểm tra thuộc tính
            // Nếu là file không có thuộc tính đặc biệt
            if (entry.attributes == 0x00)//Không có thuộc tính là file bình thường
            {
                printf("File Name: %.8s.", entry.name);
                printf("%.3s         ", entry.ext);
                printf("Position cluster: %d\n", entry.first_cluster);
            }
            if (entry.attributes == 0x10)//Là thư mục con
            {
                printf("Subdirectory name: %.8s     ", entry.name);
                printf("Position cluster: %d\n", entry.first_cluster);
            }
        }
    }
}
// Hàm hiển thị nội dung của File
void display_file_content(uint16_t cluster,const BootSector *boot_sector,char * path) {

    uint16_t element[512*(boot_sector->sectors_per_fat)*2/3];
    uint8_t FAT12 [(boot_sector->sectors_per_fat)*512];
    int32_t bytesRead = HAL_read_multi_sector(boot_sector->reserved_sectors,boot_sector->sectors_per_fat, FAT12,path);
    int num=0;
    // Chuyển đổi từ mảng uint8_t thành mảng uint16_t
    for(int m = 0; m < (512*(boot_sector->sectors_per_fat)/3); m++)
    {
        for (int n = 2*m; n< (2*m+2); n++)
        {
            num++;
            if(num==1)
            {
                element[n] = (uint16_t)(FAT12[n+m])|(uint16_t)((FAT12[n+m+1]&0x0F)<<8);
            }
            else if(num==2)
            {
                num=0;
                element[n] = (uint16_t)(FAT12[n+m]>>4)|(uint16_t)(FAT12[n+m+1]<<4);
            }
        }
    }
    uint32_t current_root = boot_sector->reserved_sectors + boot_sector->num_fats * boot_sector->sectors_per_fat;
    uint32_t root_sector =  boot_sector->root_entries *32/512;
    uint8_t sectors[SECTOR_SIZE*(boot_sector->sectors_per_cluster)];
    RootDirectory entry;
    int count=0;
    int posi_clus[10000];
    posi_clus[count]=cluster;
    //Đếm số phần tử cần đọc
    while (element[cluster] != 0x0FFF)
    {
        count ++;
        posi_clus[count]= element[cluster];
        cluster =element[cluster];
    }
    // Thực hiện in nội dung của các sector
    for (int i = 0; i <= count; i++)
    {
        bytesRead = HAL_read_multi_sector((current_root+root_sector+(posi_clus[i]-2)*(boot_sector->sectors_per_cluster)),boot_sector->sectors_per_cluster,sectors,path);
        printf("%s",sectors);
    }
}

