//-----------------------------------------------------------------------------------------     
//                 Customized Virtual File System Application
//  Programming Language:for devlop this project I am using a C programing Language as a native programming language.
//Description: This project is the virtual Represnetaion of File System.File system is considerd as the way of storing the information about files and data from the file into secondary storage device.This project is used to understand the internals and datastructures implementaion of  C programmming
// --*NOTE*-Using extension of cpp is ontly for the function overloadiing         purpose------------------//
//Operating system Used : As this project executes on primary storage tha is RAM,we can use Linux,Windows,MacOs.
//Author : Komal Kantilal Pathare                                                            
//----------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------------
//                 HEADER FILES
//--------------------------------------------------------------------------------------------- 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <fcntl.h>
#include <unistd.h>
#include <io.h>

//----------------------------------------------------------------------------------------------
//                USER-DEFINED MACRO//
//----------------------------------------------------------------------------------------------

#define MAXINODE 50//Maximum Files to be created 50
#define READ 1
#define WRITE 2 // Give permission as 3 for Both Read and Write

#define MAXFILESIZE 2048//Maximum Size of a file(1024=1kb)

#define REGULAR 1//i.e. Regular file
#define SPECIAL 2//i.e. .c,.py file

#define START 0 //File offset(lseek)
#define CURRENT 1
#define END 2 //Hole in the file i.e. potential gap

//----------------------------------------------------------------------------------------------
//                         TYPEDEF STRUCTURE:superblock
//----------------------------------------------------------------------------------------------

typedef struct superblock
{
    int TotalInodes; //initially sizze is 50 for both
    int FreeInodes;
} SUPERBLOCK, *PSUPERBLOCK;

//----------------------------------------------------------------------------------------------
//                         TYPEDEF STRUCTURE : inode
//----------------------------------------------------------------------------------------------

typedef struct  inode //86 bytes are allocated for this block
{
    char Filename[MAXINODE]; //file name stored
    int InodeNumber;//inode number
    int FileSize;//1024 byte
    int FileActualSize;//allocated when we write inot it i.e.10 bytes of data
    int FileType;//1024 byte
    char *Buffer;//on windows it stores block number but in this code it stores 1024 bytes
    int LinkCount;//linking count
    int ReferenceCount;//refernce count
    int Permission; // 1  2  3 read write permisiion 
    struct inode *next; //self referential structure

} INODE, *PINODE, **PPINODE;

//----------------------------------------------------------------------------------------------
//                     TYPEDEF STRUCTURE :filetable 
//----------------------------------------------------------------------------------------------

typedef struct filetable
{
    int redoffset;//froom where to Read
    int writeoffset;//From where to Write
    int count;//remains 1 throught the code
    int mode; // 1   2   3 mode of file 
    PINODE ptrinode;//pointer ,Linkedlist point to Inode
} FILETABLE, *PFILETABLE;

//----------------------------------------------------------------------------------------------
//                                TYPEDEF STRUCTURE : ufdt
//----------------------------------------------------------------------------------------------

typedef struct ufdt
{
    PFILETABLE ptrfiletable;//create ufdt structure,Pointer which points to file table
} UFDT;

//----------------------------------------------------------------------------------------------
//                                   GLOBAL VARIABLE
//----------------------------------------------------------------------------------------------

UFDT UFDTArr[MAXINODE];//Create array of structure i.e. Array of pointer
SUPERBLOCK SUPERBLOCKobj;//Global variable
PINODE head = NULL;//Global Pointer

//----------------------------------------------------------------------------------------------
//    FUNCTION NAME :   MAN              
//    Input         :   char                      
//    Output        :   None                     
//    Description   :   It Display the,description for each Commands
//----------------------------------------------------------------------------------------------

void man(char *name)
{
    if (name == NULL)
        return;
    if (strcmp(name, "create") == 0)
    {
        printf("Description: used to create mew regular file\n");
        printf("Usage: create file_name permission\n");
    }
    else if (strcmp(name, "read") == 0)
    {
        printf("Description: used to read data from regular file\n");
        printf("Usage: read file_name no_of_bytes_to_read\n");
    }
    else if (strcmp(name, "write") == 0)
    {
        printf("Description: used to write regular file\n");
        printf("Usage: write file_name\n after this enter the data that we want to write\n");
    }
    else if (strcmp(name, "ls") == 0)
    {
        printf("Description: used to list all information of files\n");
        printf("Usage: ls\n");
    }
    else if (strcmp(name, "stat") == 0)
    {
        printf("Description: used to display information of file\n");
        printf("Usage: stat file_name\n");
    }
    else if (strcmp(name, "fstat") == 0)
    {
        printf("Description: used to display information of file\n");
        printf("Usage: stat file_descriptor\n");
    }
    else if (strcmp(name, "truncate") == 0)
    {
        printf("Description: used to remove data from file\n");
        printf("Usage: truncate file_name\n");
    }
    else if (strcmp(name, "open") == 0)
    {
        printf("Description: used to existing file\n");
        printf("Usage: open file_name mode\n");
    }
    else if (strcmp(name, "close") == 0)
    {
        printf("Description: used to close opened file\n");
        printf("Usage: close file_name\n");
    }
    else if (strcmp(name, "closeall") == 0)
    {
        printf("Description: used to close all opened file\n");
        printf("Usage: closeall\n");
    }
    else if (strcmp(name, "lseek") == 0)
    {
        printf("Description: used to file offset\n");
        printf("Usage: lseek file_name changeoffset startpoint\n");
    }
    else if (strcmp(name, "rm") == 0)
    {
        printf("Description: used to delete file\n");
        printf("Usage: rm file_name\n");
    }
    else
    {
        printf("no manual entry available");
    }
}
//----------------------------------------------------------------------------------------------
//      FUNCTION NAME : DisplayHelp        
//      Input         : None                       
//      Output        : None                     
//      Description   : It displays all list/Operations about this application    
//----------------------------------------------------------------------------------------------

void DisplayHelp()
{
    printf("ls : to list out all file\n");
    printf("clear :to clear the console\n");
    printf("open :to open the file\n");
    printf("close : to close the file\n");
    printf("closeall :to close all file\n");
    printf("read :to read content from file\n");
    printf("write :to write the content into file\n");
    printf("exit :to terminate file system\n");
    printf("stat :to display information of file using name\n");
    printf("fstat :to display information of file using file descriptor\n");
    printf("truncate :to remove all data from file\n");
    printf("rm :to delete file\n");
    printf("backup :To take Backup of All Created files\n");
}

//----------------------------------------------------------------------------------------------
//        FUNCTION NAME :  GetFDFromName   
//        Input         : char*                     
//        Ouput         : Integer                    
//        Description   : Get File Descriptor Value
//----------------------------------------------------------------------------------------------

int GetFDFromName(char *name)
{
    int i = 0;

    while (i < 50)
    {
        if (UFDTArr[i].ptrfiletable != NULL)
            if (strcmp((UFDTArr[i].ptrfiletable->ptrinode->Filename), name) == 0)
                break;
        i++;
    }

    if (i == 50)
        return -1;
    else
        return i;
}

//----------------------------------------------------------------------------------------------
//             FUNCTION NAME : Get_Inode           
//             Input         : char*                      
//             Output        : PINODE                     
//             Description   : Return Inode VAlue of File                                 
//----------------------------------------------------------------------------------------------

PINODE Get_Inode(char *name)
{
    PINODE temp = head;
    int i = 0;

    if (name == NULL)
        return NULL;

    while (temp != NULL)
    {
        if (strcmp(name, temp->Filename) == 0)
            break;
        temp = temp->next;
    }
    return temp;
}

//----------------------------------------------------------------------------------------------
//            FUNCTION NAME : CreateDILB      
//            Input         : None                     
//            Ouput         : None                     
//            Descrption    : It creates the DILB when Program starts                       
//----------------------------------------------------------------------------------------------

void CreateDILB()
{
    int i = 1;
    PINODE newn = NULL;
    PINODE temp = head;

    while (i <= MAXINODE)
    {
        newn = (PINODE)malloc(sizeof(INODE));
        newn->LinkCount = 0;
        newn->ReferenceCount = 0;
        newn->FileType = 0;
        newn->FileSize = 0;

        newn->Buffer = NULL;
        newn->next = NULL;

        newn->InodeNumber = i;

        if (temp == NULL)
        {
            head = newn;
            temp = head;
        }
        else
        {
            temp->next = newn;
            temp = temp->next;
        }
        i++;
    }
    printf("DILB created successfully\n");
}

//---------------------------------------------------------------------------------------------
//        FUNCTION NAME : Initialisesuperblock
//         Input        : None                          
//          Output      : None                        
//          Description :Initialize Inode Values  
//----------------------------------------------------------------------------------------------

void InitialiseSuperBlock()
{
    int i = 0;
    while (i < MAXINODE)
    {
        UFDTArr[i].ptrfiletable = NULL;
        i++;
    }
    SUPERBLOCKobj.TotalInodes = MAXINODE;
    SUPERBLOCKobj.FreeInodes = MAXINODE;
}

//----------------------------------------------------------------------------------------------
//       FUNCTION NAME : CreatFile           
//               Input : char*,Integer               
//         Description : Create New Files      
//----------------------------------------------------------------------------------------------

int CreateFile(char *name, int permission)
{
    int i = 0;
    PINODE temp = head;

    if ((name == NULL) || (permission == 0) || (permission > 3))
        return -1;
    if (SUPERBLOCKobj.FreeInodes == 0)
        return -2;

    (SUPERBLOCKobj.FreeInodes)--;

    if (Get_Inode(name) != NULL)
    {
        return -3;
        (SUPERBLOCKobj.FreeInodes)++;
    }
    while (temp != NULL)
    {
        if (temp->FileType == 0)
            break;
        temp = temp->next;
    }

    while (i < MAXINODE)
    {
        if (UFDTArr[i].ptrfiletable == NULL)
            break;
        i++;
    }

    UFDTArr[i].ptrfiletable = (PFILETABLE)malloc(sizeof(FILETABLE));
    UFDTArr[i].ptrfiletable->count = 1;
    UFDTArr[i].ptrfiletable->mode = permission;
    UFDTArr[i].ptrfiletable->redoffset = 0;
    UFDTArr[i].ptrfiletable->writeoffset = 0;

    UFDTArr[i].ptrfiletable->ptrinode = temp;

    strcpy(UFDTArr[i].ptrfiletable->ptrinode->Filename, name);
    UFDTArr[i].ptrfiletable->ptrinode->FileType = REGULAR;
    UFDTArr[i].ptrfiletable->ptrinode->ReferenceCount = 1;
    UFDTArr[i].ptrfiletable->ptrinode->LinkCount = 1;
    UFDTArr[i].ptrfiletable->ptrinode->FileSize = MAXFILESIZE;
    UFDTArr[i].ptrfiletable->ptrinode->FileActualSize = 0;
    UFDTArr[i].ptrfiletable->ptrinode->Permission = permission;
    UFDTArr[i].ptrfiletable->ptrinode->Buffer = (char *)malloc(MAXFILESIZE);
    memset(UFDTArr[i].ptrfiletable->ptrinode->Buffer, 0, 1024);
    return i;
}

//----------------------------------------------------------------------------------------------
//                  FUNCTION NAME : RM_File             
//                          Input : char*                      
//                          Output: Integer                     
//                    Description : Remove Created Files 
//                        rm_file("demo.txt");
//----------------------------------------------------------------------------------------------

int rm_file(char *name)
{
    int fd = 0;

    fd = GetFDFromName(name);
    if (fd == -1)
        return -1;

    (UFDTArr[fd].ptrfiletable->ptrinode->LinkCount)--;

    if (UFDTArr[fd].ptrfiletable->ptrinode->LinkCount == 0)
        ;
    {
        UFDTArr[fd].ptrfiletable->ptrinode->FileType = 0;
        // free(UFDTArr[fd].ptrfiletable->ptrinode->Buffer);
        free(UFDTArr[fd].ptrfiletable);
    }
    strcpy(UFDTArr[fd].ptrfiletable->ptrinode->Filename, " ");
    UFDTArr[fd].ptrfiletable = NULL;
    (SUPERBLOCKobj.FreeInodes)++;
}

//----------------------------------------------------------------------------------------------
//              FUNCTION NAME :Readfile           
//                      Input : Integer , char*,Integer    
//                     Output : Integer                   
//                Description : Read Data from file   
//----------------------------------------------------------------------------------------------

int ReadFile(int fd, char *arr, int isize)
{
    int read_size = 0;
    if (UFDTArr[fd].ptrfiletable == NULL)
        return -1;
    if ((UFDTArr[fd].ptrfiletable->ptrinode->ReferenceCount) <= 1)
    {
        return -5;
    }
    if (UFDTArr[fd].ptrfiletable->mode != READ && UFDTArr[fd].ptrfiletable->mode != READ + WRITE)
        return -2;
    if (UFDTArr[fd].ptrfiletable->ptrinode->Permission != READ && UFDTArr[fd].ptrfiletable->ptrinode->Permission != READ + WRITE)
        return -2;

    if (UFDTArr[fd].ptrfiletable->redoffset == UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize)
        return -3;

    if (UFDTArr[fd].ptrfiletable->ptrinode->FileType != REGULAR)
        return -4;

    read_size = (UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize) - (UFDTArr[fd].ptrfiletable->redoffset);

    if (read_size < isize)
    {
        strncpy(arr, (UFDTArr[fd].ptrfiletable->ptrinode->Buffer) + (UFDTArr[fd].ptrfiletable->redoffset), read_size);

        UFDTArr[fd].ptrfiletable->redoffset = UFDTArr[fd].ptrfiletable->redoffset + read_size;
    }
    else
    {
        strncpy(arr, (UFDTArr[fd].ptrfiletable->ptrinode->Buffer) + (UFDTArr[fd].ptrfiletable->redoffset), isize);

        (UFDTArr[fd].ptrfiletable->redoffset) = (UFDTArr[fd].ptrfiletable->redoffset) + isize;
    }
    return isize;
}

//----------------------------------------------------------------------------------------------
//                        FUNCITON NAME : WriteFile         
//                        Input         :  Integer ,char*,Integer   
//                        Output        :     Integer              
//                        Description   : Wrtie Data Innto the file
//----------------------------------------------------------------------------------------------

int WriteFile(int fd, char *arr, int isize)
{
    if ((UFDTArr[fd].ptrfiletable->ptrinode->ReferenceCount) <= 1)
    {
        return -4;
    }
    if (((UFDTArr[fd].ptrfiletable->mode) != WRITE) && ((UFDTArr[fd].ptrfiletable->mode) != READ + WRITE))
        return -1;

    if (((UFDTArr[fd].ptrfiletable->ptrinode->Permission) != WRITE) && ((UFDTArr[fd].ptrfiletable->ptrinode->Permission) != READ + WRITE))
        return -1;

    if ((UFDTArr[fd].ptrfiletable->writeoffset) == MAXFILESIZE)
        return -2;
    if ((UFDTArr[fd].ptrfiletable->ptrinode->FileType) != REGULAR)
        return -3;

    strncpy((UFDTArr[fd].ptrfiletable->ptrinode->Buffer) + (UFDTArr[fd].ptrfiletable->writeoffset), arr, isize);

    (UFDTArr[fd].ptrfiletable->writeoffset) = (UFDTArr[fd].ptrfiletable->writeoffset) + isize;
    (UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize) = (UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize) + isize;

    return isize;
}

//----------------------------------------------------------------------------------------------
//                         FUNCTION NAME :OpenFile             
//                                 Input : char*,Integer               
//                                 Ouput : Integer                     
//                           Description : Open an Existing file 
//----------------------------------------------------------------------------------------------

int OpenFile(char *name, int mode)
{
    int i = 0;
    PINODE temp = NULL;
    if (name == NULL || mode <= 0)
        return -1;
    temp = Get_Inode(name);
    if (temp == NULL)
        return -2;

    if (temp->Permission < mode)
        return -3;

    while (i < 50)
    {
        if (UFDTArr[i].ptrfiletable == NULL)
            break;
        i++;
    }

    UFDTArr[i].ptrfiletable = (PFILETABLE)malloc(sizeof(FILETABLE));
    if (UFDTArr[i].ptrfiletable == NULL)
        return -1;
    UFDTArr[i].ptrfiletable->count = 1;
    UFDTArr[i].ptrfiletable->mode = mode;
    if (mode == READ + WRITE)
    {
        UFDTArr[i].ptrfiletable->redoffset = 0;
        UFDTArr[i].ptrfiletable->writeoffset = 0;
    }
    else if (mode == READ)
    {
        UFDTArr[i].ptrfiletable->redoffset = 0;
    }
    else if (mode == WRITE)
    {
        UFDTArr[i].ptrfiletable->writeoffset = 0;
    }

    UFDTArr[i].ptrfiletable->ptrinode = temp;
    (UFDTArr[i].ptrfiletable->ptrinode->ReferenceCount)++;

    return i;
}
 
 //--------------------------------------------------------------------------------------------
 //               FUNCTION NAME : CloseFileByName
 //---------------------------------------------------------------------------------------------

int CloseFileByName(char *name)
{
    int i = 0;
    i = GetFDFromName(name);
    if (i == -1)
        return -1;
    UFDTArr[i].ptrfiletable->redoffset = 0;
    UFDTArr[i].ptrfiletable->writeoffset = 0;
    if ((UFDTArr[i].ptrfiletable->ptrinode->ReferenceCount) > 0)
    {
        (UFDTArr[i].ptrfiletable->ptrinode->ReferenceCount)--;
    }
    return 0;
}

//----------------------------------------------------------------------------------------------
//                        FUNCTION NAME : CloseAllFile        
//                                Input : Integer                     
//                                Output: None                        
//                           Description:Close all Existing file
//----------------------------------------------------------------------------------------------

void CloseAllFile()
{
    int i = 0;
    while (i < 50)
    {
        if (UFDTArr[i].ptrfiletable != NULL)
        {
            UFDTArr[i].ptrfiletable->redoffset = 0;
            UFDTArr[i].ptrfiletable->writeoffset = 0;
            if ((UFDTArr[i].ptrfiletable->ptrinode->ReferenceCount) > 0)
            {
                (UFDTArr[i].ptrfiletable->ptrinode->ReferenceCount)--;
            }
        }
        i++;
    }
}

//----------------------------------------------------------------------------------------------
//              FUNCTION NAME : Lseekfile        
//                     Input  : Integer ,Integer ,Integer
//                   Ouput    :   Integer               
//               Description  :   write data into the file from particular position       
//----------------------------------------------------------------------------------------------

int Lseekfile(int fd, int size, int from)
{
    if ((fd < 0) || (from > 2))
        return -1;

    if (UFDTArr->ptrfiletable == NULL)
        return -1;

    if ((UFDTArr[fd].ptrfiletable->mode == READ) || (UFDTArr->ptrfiletable->mode == READ + WRITE))
    {
        if (from == CURRENT)
        {
            if (((UFDTArr->ptrfiletable->redoffset) + size) > UFDTArr->ptrfiletable->ptrinode->FileActualSize)
                return -1;
            if (((UFDTArr[fd].ptrfiletable->redoffset) + size) < 0)
                return -1;
            (UFDTArr[fd].ptrfiletable->redoffset) = (UFDTArr[fd].ptrfiletable->redoffset) + size;
            size;
        }

        else if (from == START)
        {
            if (size > (UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize))
                return -1;
            if (size < 0)
                return -1;
            (UFDTArr[fd].ptrfiletable->redoffset) = size;
        }
        else if (from == END)
        {
            if ((UFDTArr->ptrfiletable->ptrinode->FileActualSize) + size > MAXFILESIZE)
                return -1;
            if (((UFDTArr[fd].ptrfiletable->redoffset) + size) < 0)
                return -1;
            (UFDTArr[fd].ptrfiletable->redoffset) = (UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize) + size;
        }
    }
    else if (UFDTArr[fd].ptrfiletable->mode == WRITE)
    {
        if (from == CURRENT)
        {
            if (((UFDTArr[fd].ptrfiletable->writeoffset) + size) > MAXFILESIZE)
                return -1;
            if (((UFDTArr[fd].ptrfiletable->writeoffset) + size) < 0)
                return -1;
            if (((UFDTArr[fd].ptrfiletable->writeoffset) + size) > (UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize))
                ;
            (UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize) = (UFDTArr[fd].ptrfiletable->writeoffset) + size;
            (UFDTArr[fd].ptrfiletable->writeoffset) = (UFDTArr[fd].ptrfiletable->writeoffset) + size;
        }
        else if (from == START)
        {
            if (size > MAXFILESIZE)
                return -1;
            if (size < 0)
                return -1;
            if (size > (UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize))
                (UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize) = size;
            (UFDTArr[fd].ptrfiletable->writeoffset) = size;
        }
        else if (from == END)
        {
            if ((UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize) + size > MAXFILESIZE)
                return -1;
            if (((UFDTArr[fd].ptrfiletable->writeoffset) + size) < 0)
                return -1;
            (UFDTArr[fd].ptrfiletable->writeoffset) = (UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize) + size;
        }
    }
}

//----------------------------------------------------------------------------------------------
//                   FUNCTION NAME : ls_file           
//                           Input :   None                    
//                       Output    :   None                
//                   Description   :   List out all existion files Name                 
//---------------------------------------------------------------------------------------------

void ls_file()
{
    int i = 0;
    PINODE temp = head;
    if (SUPERBLOCKobj.FreeInodes == MAXINODE)
    {
        printf("error:there are no files\n");
        return;
    }

    printf("\n file name\t Inode number\tFile size\tLink count\n");
    printf("------------------------------------------------------\n");
    while (temp != NULL)
    {
        if (temp->FileType != 0)
        {
            printf("%s\t\t%d\t\t%d\t\t%d\n", temp->Filename, temp->InodeNumber, temp->FileActualSize, temp->LinkCount);
        }
        temp = temp->next;
    }
    printf("------------------------------------------------------\n");
}

//----------------------------------------------------------------------------------------------
//                  FUNCTION NAME : save_data
//----------------------------------------------------------------------------------------------

void save_data()
{
    int i = 0, fd1, fd2, size = 0, length = 0;
    char container[1024], fname[30];
    PINODE temp = head;
    while (1)
    {
        strcpy(fname, temp->Filename);
        fd1 = creat(fname, 0777);
        open(fname, O_RDWR);
        fd2 = GetFDFromName(fname);
        length = UFDTArr[fd2].ptrfiletable->ptrinode->FileActualSize;
        while ((size = ReadFile(fd2, container, length)) != 0)
        {
            write(fd1, container, size);
            break;
        }
        temp = temp->next;
        if (UFDTArr[fd2 + 1].ptrfiletable == NULL)
        {
            break;
        }
    }
}

//----------------------------------------------------------------------------------------------
//                          FUNCTION NAME : fstat_file
//----------------------------------------------------------------------------------------------

int fstat_file(int fd)
{
    PINODE temp = head;
    int i = 0;
    if (fd < 0)
        return -1;
    if (UFDTArr[fd].ptrfiletable == NULL)
        return -2;
    temp = UFDTArr[fd].ptrfiletable->ptrinode;
    printf("\n---------- statical informantion about file--------\n");
    printf("filename:%s\n", temp->Filename);
    printf("Inode Number:%d\n", temp->InodeNumber);
    printf("find size:%d\n", temp->FileSize);
    printf("actual file size:%d\n", temp->FileActualSize);
    printf("link count:%d\n", temp->LinkCount);
    printf("references count:%d\n", temp->ReferenceCount);

    if (temp->Permission == 1)
    {
        printf("file permission:read only\n");
    }
    else if (temp->Permission == 2)
    {
        printf("file permission:write\n");
    }
    else if (temp->Permission == 3)
    {
        printf("file permission:read and write\n");
    }
    printf("------------------------------------------------------\n");
    return 0;
}

//----------------------------------------------------------------------------------------------
//                  FUNCTION NAME : stat_file
//----------------------------------------------------------------------------------------------

int stat_file(char *name)
{
    PINODE temp = head;
    int i = 0;
    if (name == NULL)
        return -1;
    while (temp != NULL)
    {
        if (strcmp(name, temp->Filename) == 0)
            break;
        temp = temp->next;
    }

    if (temp == NULL)
        return -2;
    printf("\n---------- statical informantion about file--------\n");
    printf("filename:%s\n", temp->Filename);
    printf("Inode Number:%d\n", temp->InodeNumber);
    printf("find size:%d\n", temp->FileSize);
    printf("actual file size:%d\n", temp->FileActualSize);
    printf("link count:%d\n", temp->LinkCount);
    printf("references count:%d\n", temp->ReferenceCount);
    if (temp->Permission == 1)
    {
        printf("file permission:read only\n");
    }
    else if (temp->Permission == 2)
    {
        printf("file permission:writen\n");
    }
    else if (temp->Permission == 3)
    {
        printf("file permission:read and write\n");
    }
    printf("------------------------------------------------------\n");
    return 0;
}

//----------------------------------------------------------------------------------------------
//                            FUNCTION NAME : Truncate_file
//----------------------------------------------------------------------------------------------

int Truncate_file(char *name)
{
    int fd = GetFDFromName(name);
    if (fd == -1)
        return -1;

    memset(UFDTArr[fd].ptrfiletable->ptrinode->Buffer, 0, 1024);
    UFDTArr[fd].ptrfiletable->redoffset = 0;
    UFDTArr[fd].ptrfiletable->writeoffset = 0;
    UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize = 0;
}

//----------------------------------------------------------------------------------------------
 //                      MAIN FUNCTION                 
 //                      Input: None                      
 //                     Output:Integer                    
 //               Description :  Entry point funtion 
 //---------------------------------------------------------------------------------------------

int main()
{
    char *ptr = NULL;
    int ret = 0, fd = 0, count = 0;
    char command[4][80], str[80], arr[1024];

    InitialiseSuperBlock();
    CreateDILB();

    while (1)
    {
        fflush(stdin);
        strcpy(str, "");
        printf("VFS:>");
        fgets(str, 80, stdin);
        count = sscanf(str, "%s %s %s %s", command[0], command[1], command[2], command[3]);

        if (count == 1)
        {
            if (strcmp(command[0], "ls") == 0)
            {
                ls_file();
            }
            else if (strcmp(command[0], "closeall") == 0)
            {
                CloseAllFile();
                printf("all files are successfully close\n");
            }
            else if (strcmp(command[0], "clear") == 0)
            {
                system("clear");
                continue;
            }

            else if (strcmp(command[0], "help") == 0)
            {
                DisplayHelp();
                continue;
            }
            else if (strcmp(command[0], "exit") == 0)
            {

                save_data();
                CloseAllFile();
                printf("terminating vartual file system project\n");
                exit(0);
            }
            else
            {
                printf("ERROR:command not found\n");
                continue;
            }
        }

        else if (count == 2)
        {
            if (strcmp(command[0], "stat") == 0)
            {
                ret = stat_file(command[1]);
                if (ret == -1)
                    printf("ERROR: incorrect parameter\n");
                if (ret == -2)
                    printf("ERROR: there is no such element\n");
                continue;
            }
            else if (strcmp(command[0], "fstat") == 0)
            {
                ret = fstat_file(atoi(command[1]));
                if (ret == -1)
                    printf("ERROR: incorrect parameter\n");
                if (ret == -2)
                    printf("ERROR: there is no such element\n");
                continue;
            }
            else if (strcmp(command[0], "close") == 0)
            {
                ret = CloseFileByName(command[1]);
                if (ret == -1)
                    printf("ERROR: there is no such element\n");
                continue;
            }
            else if (strcmp(command[0], "rm") == 0)
            {
                ret = rm_file(command[1]);
                if (ret == -1)
                    printf("ERROR: there is no such element\n");
                continue;
            }
            else if (strcmp(command[0], "man") == 0)
            {
                man(command[1]);
            }
            else if (strcmp(command[0], "write") == 0)
            {
                fd = GetFDFromName(command[1]);
                if (ret == -1)
                {
                    printf("ERROR: incorrect parameter\n");
                    continue;
                }
                printf("enter the data:\n");
                scanf("%[^'\n']", arr);
                ret = strlen(arr);
                if (ret == 0)
                {
                    printf("ERROR: incorrect parameter\n");
                    continue;
                }

                ret = WriteFile(fd, arr, ret);
                if (ret == -1)
                    printf("ERROR: permission denied\n");
                if (ret == -2)
                    printf("ERROR: there is no sufficient memory to write\n");
                if (ret == -3)
                    printf("ERROR: it is not regular files\n");
                if (ret == -4)
                    printf("ERROR: file not open\n");
                else
                {
                    continue;
                }
            }

            else if (strcmp(command[0], "truncate") == 0)
            {
                ret = Truncate_file(command[1]);
                if (ret == -1)
                    printf("ERROR: incorrect parameter\n");
            }
            else
            {
                printf("ERROR: command not found!\n");
                continue;
            }
        }

        else if (count == 3)
        {
            if (strcmp(command[0], "create") == 0)
            {
                ret = CreateFile(command[1], atoi(command[2]));
                if (ret >= 0)
                    printf("file successfully created with description:%d\n");
                if (ret == -1)
                    printf("ERROR:incorrect password\n");
                if (ret == -2)
                    printf("ERROR:file not presnt\n");
                if (ret == -3)
                    printf("ERROR:permission denied\n");
                continue;
            }
            else if (strcmp(command[0], "open") == 0)
            {
                ret = OpenFile(command[1], atoi(command[2]));
                if (ret >= 0)
                {
                    printf("file successfully open with file descriptor:%d", ret);
                }
                if (ret == -1)
                {
                    printf("error:incorrect parameter");
                }
                if (ret == -2)
                {
                    printf("error:file not present");
                }
                if (ret == -3)
                {
                    printf("error: permisson denied");
                }
                continue;
            }

            else if (strcmp(command[0], "read") == 0)
            {
                fd = GetFDFromName(command[1]);
                if (fd == -1)
                {
                    printf("ERROR:incorrect password\n");
                    continue;
                }

                ptr = (char *)malloc(sizeof(atoi(command[2])) + 1);
                if (ptr == NULL)
                {
                    printf("ERROR: memory allocation failure\n");
                    continue;
                }
                ret = ReadFile(fd, ptr, atoi(command[2]));
                if (ret == -1)
                    printf("ERROR:file not exits\n");
                if (ret == -2)
                    printf("ERROR:permission denied\n");
                if (ret == -3)
                    printf("ERROR:reaches at end of file\n");
                if (ret == -4)
                    printf("ERROR:it is nor regular file\n");
                if (ret == -5)
                    printf("ERROR: file not open\n");
                if (ret == 0)
                    printf("ERROR:file empty\n");
                if (ret > 0)
                {
                    write(2, ptr, ret);
                }
                continue;
            }
            else
            {
                printf("ERROR: command not found\n");
                continue;
            }
        }
        else if (count == 4)
        {
            if (strcmp(command[0], "lseek") == 0)
            {
                fd = GetFDFromName(command[1]);
                if (fd == -1)
                {
                    printf("ERROR:incorrect parameter\n");
                    continue;
                }
                ret = Lseekfile(fd, atoi(command[2]), atoi(command[3]));
                if (ret == -1)
                {
                    printf("ERROR:unable to perform lseek\n");
                }
            }
            else
            {
                printf("ERROR:command not found!!!\n");
                continue;
            }
        }
        else
        {
            printf("ERROR:command not found!!!!!!!!!\n");
            continue;
        }
    }
    return 0;
}
//-----------------------------code ended-----------------------------------------------------//