/*
  Ahnaf Ahmad
  1001835014
*/
// The MIT License (MIT)
// 
// Copyright (c) 2020 Trevor Bakker 
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.


// 1 relative path
// 2 compare
// 3 read printing the right stuff

#define _GNU_SOURCE

#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <ctype.h>

#include <stdint.h>

#define MAX_NUM_ARGUMENTS 4

#define WHITESPACE " \t\n"      // We want to split our command line up into tokens
                                // so we need to define what delimits our tokens.
                                // In this case  white space
                                // will separate the tokens on our command line

#define MAX_COMMAND_SIZE 255    // The maximum command-line size

//functions
int LBAToOffset(int32_t sector);
int16_t NextLB(uint32_t sector);
int compare (char *input1, char *IMG_Name1);
void initTrash();

//structs
struct __attribute__((__packed__)) DirectoryEntry {
  char DIR_Name[11];
  uint8_t DIR_Attr;
  uint8_t Unused1[8];
  uint16_t DIR_FirstClusterHigh;
  uint8_t Unused2[4];
  uint16_t DIR_FirstClusterLow;
  uint32_t Dir_FileSizel;
};


//global variables
FILE *fp;
struct DirectoryEntry dir[16];
uint8_t delAtr;
uint8_t buffer[512];

char trash[16][11];
uint8_t deletedAttr[16];
int univIndex;


int16_t BPB_BytsPerSec;
int8_t BPB_SecPerClus;
int16_t BPB_RsvdSecCnt;
int8_t BPB_NumFATs;
int32_t BPB_FATSz32;
int cluster;

int main()
{

  univIndex=0;
  initTrash(); //initializes trash (the realm for the deleted files)

  char * cmd_str = (char*) malloc( MAX_COMMAND_SIZE );
  
  while( 1 )
  {
    // Print out the mfs prompt
    printf ("mfs> ");

    // Read the command from the commandline.  The
    // maximum command that will be read is MAX_COMMAND_SIZE
    // This while command will wait here until the user
    // inputs something since fgets returns NULL when there
    // is no input
    while( !fgets (cmd_str, MAX_COMMAND_SIZE, stdin) );

    /* Parse input */
    char *token[MAX_NUM_ARGUMENTS];

    int   token_count = 0;                                 
                                                           
    // Pointer to point to the token
    // parsed by strsep
    char *arg_ptr;                                         
                                                           
    char *working_str  = strdup( cmd_str );                

    // we are going to move the working_str pointer so
    // keep track of its original value so we can deallocate
    // the correct amount at the end
    char *working_root = working_str;
    
    // Tokenize the input stringswith whitespace used as the delimiter
    while ( ( (arg_ptr = strsep(&working_str, WHITESPACE ) ) != NULL) && 
              (token_count<MAX_NUM_ARGUMENTS))
    {
      token[token_count] = strndup( arg_ptr, MAX_COMMAND_SIZE );
      if( strlen( token[token_count] ) == 0 )
      {
        token[token_count] = NULL;
      }
        token_count++;
    }

    // Now print the tokenized input as a debug check
    // \TODO Remove this code and replace with your FAT32 functionality
    /*
    int token_index  = 0;
    for( token_index = 0; token_index < token_count; token_index ++ ) 
    {
      printf("token[%d] = %s\n", token_index, token[token_index] );  
    }
    */

    
    if(token[0] ==NULL){
        //if the user doesn't type anything, nothing happens
    }
    //terminates the mfs
    else if(strcmp(token[0],"quit")==0){
      break;
    }
    //command "open" - opens fat32 image
    else if(strcmp(token[0],"open")==0){
        if(fp==NULL){
          fp = fopen(token[1],"r");
          if(fp==NULL){
            printf("Error: File system image not found\n");
          }
          else{
            
            fseek(fp, 11,SEEK_SET);
            fread(&BPB_BytsPerSec, 2, 1,fp);

            fseek(fp,13,SEEK_SET);
            fread(&BPB_SecPerClus, 1, 1,fp);

            fseek(fp,14,SEEK_SET);
            fread(&BPB_RsvdSecCnt, 2, 1,fp);

            fseek(fp, 16, SEEK_SET);
            fread(&BPB_NumFATs, 1, 1,fp);

            fseek(fp,36,SEEK_SET);
            fread(&BPB_FATSz32, 4, 1,fp);

            cluster = (BPB_NumFATs * BPB_FATSz32 * BPB_BytsPerSec) + (BPB_RsvdSecCnt *BPB_BytsPerSec);

            fseek(fp, cluster, SEEK_SET);
            fread(dir, sizeof(struct DirectoryEntry),16,fp);
            
          }
        }
        else{
          printf("Error: File system image already open.\n");
        }
    }
    //command "close" - will close the file
    else if(strcmp(token[0],"close")==0){
      if(fp!=NULL){
        fclose(fp);
        fp=NULL;
      }
      else{
        printf("Error: File system not open.\n");
      }
    
    }

    //comand "ls" - prints directory
    else if(strcmp(token[0],"ls")==0){
        int i;
        if(fp!=NULL){
          
          for(i=0;i<16;i++){
            if(dir[i].DIR_Attr == 0x01 || dir[i].DIR_Attr == 0x10 || dir[i].DIR_Attr == 0x20 && dir[i].DIR_Attr != 0xe5 ){
              char name[12];
              memcpy(name,dir[i].DIR_Name,11);
              name[11] = '\0';
              printf("%s\n", name);
            }
          }

        }
        else{
          printf("Error: File system not open.\n");
        }
        
    }
    //command "cd" - redirects directory
    else if(strcmp(token[0],"cd")==0){
        
      int i;
      int success=0;
    
      if(fp!=NULL){
        char *directory;
        directory = strtok(token[1],"/");
        for(i=0;i<16;i++){
         // printf("%d, %s %d\n",i, dir[i].DIR_Name,dir[i].DIR_Attr);
          if(strcmp(token[1],"..")==0){

          }
          else if(strcmp(token[1],".")==0){
            success=1;
            break;
          }
          if(compare(token[1],dir[i].DIR_Name)){
            int cluster = dir[i].DIR_FirstClusterLow;
            if(cluster ==0){
              cluster =2;
            }
            int offset =LBAToOffset(cluster);
            fseek(fp, offset, SEEK_SET);
            fread(dir, sizeof( struct DirectoryEntry),16, fp );
            success=1;
            break;
          }
          
        }
        
        if(success==0){ //if file was not found
          printf("Error: File not found.\n");
        }

      }
      else{
          printf("Error: File system not open.\n");
      }
      
      
    }

    //command "info" - will print out info
    else if(strcmp(token[0],"info")==0){

      if(fp!=NULL){
        printf("BPB_BytsPerSec: %d, 0x%X\n",BPB_BytsPerSec,BPB_BytsPerSec);
        printf("BPB_SecPerClus: %d, 0x%X\n",BPB_SecPerClus,BPB_SecPerClus);
        printf("BPB_RsvdSecCnt: %d, 0x%X\n",BPB_RsvdSecCnt,BPB_RsvdSecCnt);
        printf("BPB_NumFATs:    %d, 0x%X\n",BPB_NumFATs,BPB_NumFATs);
        printf("BPB_FATSz32:    %d, 0x%X\n",BPB_FATSz32,BPB_FATSz32);
      }
      else{
        printf("Error: File system not open.\n");
      }

      
    }

    //command "stat" - shows stat of the file
    else if(strcmp(token[0],"stat")==0){
      
      int i;
      int success=0;
      if(fp!=NULL){
        for(i=0;i<16;i++){
         {
           //printf("%d: %d\n",i,compare(token[1],dir[i].DIR_Name));
           if(compare(token[1],dir[i].DIR_Name)){
            printf("Name:                    %s\n",dir[i].DIR_Name);
            printf("Attribute:               %d\n",dir[i].DIR_Attr);
            printf("Starting Cluster number: %d\n",dir[i].DIR_FirstClusterLow);
            printf("Size:                    %d\n",dir[i].Dir_FileSizel);
            success=1;
            break;
           }
    
          }
        }
        if(success==0){
          printf("Error: File not found.\n");
        }
      }
      else{
        printf("Error: File system not open.\n");
      }
      
    }

    //command "read" - reads the ascii values of teh file
    else if(strcmp(token[0],"read")==0){
      int i;
      int pos = atoi(token[2]);
      int numBytes = atoi(token[3]);
      int success=0;

      if(fp!=NULL){

        for(i=0;i<16;i++){
          if(compare(token[1],dir[i].DIR_Name)){
            
            int offset = LBAToOffset(dir[i].DIR_FirstClusterLow);
            fseek(fp, offset,SEEK_SET);
            //fseek(fp, offset + pos,SEEK_SET);
            fseek(fp, pos, SEEK_CUR);
            fread(buffer,numBytes,1,fp);
            
            for(int j=0; j<numBytes;j++){
              printf("%d ",buffer[j]);
            }
            success=1;
            break;
          }
        }
        if(success==1){
          printf("\n");
        }
        else{
          printf("Error: File not found.\n");
        }
        
      }
      else{
        
        printf("Error: File system not open.\n");
      
      }
    }

    //command "get" - gets the file from the fat32 image and places it in a working directory
    else if(strcmp(token[0],"get")==0){
      int i;
      int success=0;

      if(fp!=NULL){
        for(i=0; i<16;i++){
          if(compare(token[1],dir[i].DIR_Name)){
            int size = dir[i].Dir_FileSizel;
            int offset = LBAToOffset(dir[i].DIR_FirstClusterLow);
            fseek(fp, offset,SEEK_SET);

            FILE *ofp = fopen(token[1],"w");

            while(size >= BPB_BytsPerSec){
              fread(buffer,512,1,fp);
              fwrite(buffer,512,1,ofp);

              size = size - BPB_BytsPerSec;
              int cluster2 = NextLB(cluster);
              if(cluster2 > -1){
                offset = LBAToOffset(cluster2);
                fseek(fp,offset,SEEK_SET);
              }
              
            }

            if(size > 0){
              fread(buffer,size,1,fp);
              fwrite(buffer,size,1,ofp);
            }
            fclose(ofp);

            success=1;
            break;
          }
        }
        if(success==0){
          printf("Error: File not found.\n");
        }

      }
      else{
        printf("Error: File system not open.\n");
      }
    }

    //command "del" - deletes, specifically hides a file from fat32
    else if(strcmp(token[0],"del")==0){
      int i;
      int success=0;
      if(fp!=NULL){
        for(i=0;i<16;i++){
          if(compare(token[1],dir[i].DIR_Name)){
          
            //delAtr = dir[i].DIR_Attr;
            if(strcmp(trash[univIndex],"EMPTY")!=0){
              for(int j=0;j<16;j++){
                if(strcmp(trash[j],"EMPTY")==0){
                  strcpy(trash[j],dir[i].DIR_Name);
                  deletedAttr[j] = dir[i].DIR_Attr;
                  //break;
                }
              }
            }
            else{
              strncpy(trash[univIndex],dir[i].DIR_Name,11);
              deletedAttr[univIndex] = dir[i].DIR_Attr;
              univIndex++;
            }

            dir[i].DIR_Attr = 0xe5;
            success=1;
            break;
          }
        }
        if(success==0){
          printf("Error: File not found.\n");
        }
      }
      else{
        printf("Error: File system not open.\n");
      }

    }
    //command "undel" - undeletes/unhides the file from the directory
    else if(strcmp(token[0],"undel")==0){
      int i;
      int success=0;

      if(fp!=NULL){
        for(i=0;i<16;i++){
          //searching for the previously deleted file, which is hidden under the attr 0xe5
          if(compare(token[1],dir[i].DIR_Name)==1 && dir[i].DIR_Attr ==0xe5){ 
            
            //dir[i].DIR_Attr = delAtr;
            for(int j=0;j<16;j++){
              
              if(strncmp(trash[j],dir[i].DIR_Name,11)==0){
                //printf("hit\n");
                dir[i].DIR_Attr = deletedAttr[j];
                strcpy(trash[j],"EMPTY");
              }
            }
            success=1;
            break;
          }
        }
        if(success==0){
          printf("Error: File not found.\n");
        }

      }
      else{
        printf("Error: File system not open.\n");
      }  
      
    }
    else{
      printf("Error: Command not found\n");
    }

    free( working_root );

  }
  return 0;
}

int LBAToOffset(int32_t sector){
  return ((sector -2)*BPB_BytsPerSec) + (BPB_BytsPerSec * BPB_RsvdSecCnt) + (BPB_NumFATs * BPB_FATSz32 *BPB_BytsPerSec);
}

int16_t NextLB(uint32_t sector){
  uint32_t FATAddress = (BPB_BytsPerSec * BPB_RsvdSecCnt) + (sector*4);
  int16_t val;
  fseek(fp,FATAddress,SEEK_SET);
  fread(&val,2,1,fp);
  return val;
}

int compare (char *input1, char *IMG_Name1){

  
  
  char IMG_Name[11];
  char input[11];
  char expanded_name[12];

  strncpy(IMG_Name,IMG_Name1,11);
  strncpy(input,input1,11);

   if(strncmp(input1,"..",2)==0){
    strncpy(expanded_name,"..",2);
    expanded_name[3] = '\0';

    if( strncmp( expanded_name, IMG_Name, 2 ) == 0 )
    {
      return 1;
    }
    return 0;
  }

  memset( expanded_name, ' ', 12 );
  char *token = strtok( input, "." );
  strncpy( expanded_name, token, strlen( token ) );
  token = strtok( NULL, "." );
  if( token )
  {
    strncpy( (char*)(expanded_name+8), token, strlen(token ) );
  }
  
  expanded_name[11] = '\0';

  int i;
  for( i = 0; i < 11; i++ )
  {
    expanded_name[i] = toupper( expanded_name[i] );
  }
  
  if( strncmp( expanded_name, IMG_Name, 11 ) == 0 )
  {
    return 1;
  }
  return 0;
}

void initTrash(){

  for(int i=0;i<16;i++){
    strcpy(trash[i],"EMPTY");
  }
}
