#include<stdio.h>
#include"FAT12.h"


int main(){
    char path[100];
    printf("File System Name(FAT12):");
    scanf("%s", path);
    printf("------------------------------------------------------------------------------\n\n");

    BootSector boot_sector;
      int32_t bytesRead = HAL_read_sector(0, (uint8_t *)&boot_sector,path); // Read boot sector from sector 0

    if (bytesRead < 0) {
        printf("Error reading boot sector.\n");
        return 1;
    }
    print_boot_sector_info(&boot_sector);

    printf("------------------------------------------------------------------------------\n");
    printf("List all files and folders in the root directory\n");
    print_directory_in_root(&boot_sector,path);

    printf("-------------------------------------------------------------------------------\n");
    printf("Virtual File System\n");
    printf("1. List all files and folders in subdirectory\n");
    printf("2. Display contents of a file\n");
    printf("3. Exit\n");
    int choice;
    while (1) {
        printf("\nEnter your choice: ");
        scanf("%d", &choice);

        switch (choice) {
            case 1:
                printf("\nEnter the name of the folder : ");
                char foldername[256]; int po_clust;
                scanf("%s", foldername);
                printf("\nPosition folder cluster : ");
                scanf("%d", &po_clust);
                list_files_in_directory(po_clust,&boot_sector,path);
                break;
            case 2:
                printf("\nEnter the name of the file to display: ");
                char filename[256]; int po_clus;
                scanf("%s", filename);
                printf("\nPosition file cluster : ");
                scanf("%d", &po_clus);
                display_file_content(po_clus,&boot_sector,path);
                break;
            case 3:
                printf("\nExiting...\n");
                return 0;
            default:
                printf("\nInvalid choice. Please try again.\n");
                break;
        }
    }

      return  0;
}
