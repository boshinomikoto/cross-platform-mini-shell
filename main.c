#include <stdio.h>
#include <stdbool.h> 
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdint.h>

#ifdef _WIN32
    #include <windows.h>
    #define CLEAR_SCREEN "cls"
#else
    #include <dirent.h>
    #include <sys/stat.h>
    #include <unistd.h> // getcwd
    #include <limits.h> // PATH_MAX
    #define CLEAR_SCREEN "\033[2J\033[1;1H"
#endif

//auxiliary functions
void ParseString(char* str);
void DelSpace(char* str, int commandSize);
char* SplitString(int size, char* str, char dest[]);
void ShowBytes(char* filename);
void FilesTree();

//program functional 
uint64_t get_file_size(const char* path);
void ReadBinFiles(const char* filename);
void CreateFiles(const char* filename);
void DeleteFiles(const char* filename);
void ShowInConsole(const char* filename);
void ChangeDirectory(char* path);
void MakeDirectory(const char* str);
void PrintCurrentPath();
char* GetCurrentPath(void);
void Clear();
void List();
void print_logo();
void Help();

//sv
void Sv_Info();
int Sv_Sh();

int main()
{
#ifdef _WIN32
    SetConsoleOutputCP(1251);
    SetConsoleCP(1251);
#endif

    char* s = GetCurrentPath();
    if (!s)
    {
        fprintf(stderr, "Failed to get path\n");
        return 1;
    }

  print_logo();
  char parsedString[256];

  while (true) 
  {
    s = GetCurrentPath();
    printf("\033[38;5;%dmsv@[%s]> \033[m", 226, s);
    if (!fgets(parsedString, sizeof(parsedString), stdin))
        break;
    parsedString[strcspn(parsedString, "\n")] = '\0';
    ParseString(parsedString);
  }  
  free(s);
  return 0;
}
 //не нравится что непооучится работать с файлом если есть пробелы так как фуукнция delspace удаляет их
 //формирования строки dest очень не удобная
void ParseString(char* str)
{
    if(str == NULL) return;
    int commandSize = 0;
    while(str[commandSize] != ' ' && str[commandSize] != '\0')
        commandSize++;
    size_t len = strlen(str);
    if(commandSize > len) return;

    DelSpace(str, commandSize);
    char dest[(strlen(str) - commandSize) + 1];
    
    if (strcmp(str, "ls") == 0) List(); //ls
    else if (strcmp(str, "help") == 0) Help(); //help
    else if (strcmp(str, "exit") == 0) exit(0); //exit
    else if (strcmp(str, "sv-info") == 0) Sv_Info(); //sv-info
    else if (strcmp(str, "sv-sh") == 0) Sv_Sh(); //sv-sh
    else if (strcmp(str, "cl") == 0) Clear(); //cl
    else if (strcmp(str, "cwd") == 0) PrintCurrentPath(); //cwd
    else if (strncmp(str, "dir", 3) == 0) //dir
    {
        MakeDirectory(SplitString(commandSize, str, dest));
    }
    else if (strncmp(str, "cd", 2) == 0) //cd
    {
        ChangeDirectory(SplitString(commandSize, str, dest));
    }
    else if (strncmp(str, "cat", 3) == 0) //cat
    {
        ShowInConsole(SplitString(commandSize, str, dest));
    }
    else if (strncmp(str, "cf", 2) == 0) //cf 
    {
        CreateFiles(SplitString(commandSize, str, dest));
    }
    else if (strncmp(str, "bin", 3) == 0) //bin
    {
        ReadBinFiles(SplitString(commandSize, str, dest));
    }
    else if (strncmp(str, "s", 1) == 0) //size
    {
        ShowBytes(SplitString(commandSize, str, dest));
    }
    else if (strncmp(str, "rm", 2) == 0) //remove
    {
        DeleteFiles(SplitString(commandSize, str, dest));
    }
    else if (strncmp(str, "tree", 4) == 0) FilesTree(); //tree

    else if (strcmp(str, "") == 0) printf("%s", "");
    else printf("Unknown command\n");
}
void DelSpace(char* str, int commandSize)
{
    int i = commandSize;
    // ищем первый пробел после команды
    while (str[i] != ' ' && str[i] != '\0')
        i++;
    if (str[i] == '\0') return; // нет пробела
    
    // сдвигаем всё что после пробела
    int j = i;
    i++;
    while (str[i] != '\0')
        str[j++] = str[i++];
    str[j] = '\0';
}

char* SplitString(int commandSize, char* str, char dest[])
{
    int i = commandSize;
    int j = 0;
   
    for ( ; str[i] != '\0'; i++)
    {
        dest[j] = str[i];
        j++;
    }
    dest[j] = '\0';

    if (dest[0] == '\0') printf("No arguments");
    
    return dest;
}
void ShowBytes(char* filename)
{
    uint64_t size = get_file_size(filename);
    if (size == (uint64_t)-1)
        printf("Error\n");
    else
        printf("%s %llu bytes\n", filename, (unsigned long long)size);
}
void FilesTree()
{
    char* s = GetCurrentPath();
    List();

#ifdef _WIN32
    WIN32_FIND_DATA findData;
    HANDLE hFind = FindFirstFile("*", &findData);

    if (hFind == INVALID_HANDLE_VALUE)
    {
        free(s);
        return;
    }
    do
    {
        if ((findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) &&
            strcmp(findData.cFileName, ".") != 0 &&
            strcmp(findData.cFileName, "..") != 0)
        {
            ChangeDirectory(findData.cFileName);
            FilesTree();  // рекурсивный вызов
            ChangeDirectory(".."); 
        }
    } while (FindNextFile(hFind, &findData));

    FindClose(hFind);
#else
    DIR* dir = opendir(".");
    if (!dir) { free(s); return; }

    struct dirent* entry;
    while ((entry = readdir(dir)) != NULL)
    {
        struct stat st;
        if (stat(entry->d_name, &st) == 0 &&
            S_ISDIR(st.st_mode) &&
            strcmp(entry->d_name, ".") != 0 &&
            strcmp(entry->d_name, "..") != 0)
        {
            ChangeDirectory(entry->d_name);
            FilesTree();
            ChangeDirectory("..");
        }
    }
    closedir(dir);
#endif
    free(s);
}

uint64_t get_file_size(const char* filename)
{
#ifdef _WIN32
    WIN32_FILE_ATTRIBUTE_DATA info;
    if (!GetFileAttributesExA(filename, GetFileExInfoStandard, &info))
        return (uint64_t)-1;
    return ((uint64_t)info.nFileSizeHigh << 32) | (uint64_t)info.nFileSizeLow;
#else
    struct stat st;
    if (stat(filename, &st) != 0)
        return (uint64_t)-1;
    return (uint64_t)st.st_size;
#endif 
}
void ReadBinFiles(const char* filename)
{
    int c;
    FILE* fp = fopen(filename, "rb");
    if (fp)
    {
        while ((c = getc(fp)) != EOF)
            printf("%c", (char)c);
        printf("%c\n", '\n');
        fclose(fp);
    }
    else
    {
        printf("Сheck if the file exists and correctness of the file name\n");
        printf("Use name for files without spaces.\n");
    }
}
void CreateFiles(const char* filename)
{
    FILE* fp = fopen(filename, "w+");
    fclose(fp);
}
void DeleteFiles(const char* filename)
{
    if (remove(filename) == 0)
        printf("%s is deleted\n", filename);
    else
        printf("%s: Cannot to delete it. Error\n", filename);
}
void ShowInConsole(const char* filename)
{
    char c;
    FILE* fp = fopen(filename, "r");
    if (fp)
    {
        while ((c = getc(fp)) != EOF)
            printf("%c", c);
        fclose(fp);
    }
    else
    {
        printf("Сheck if the file exists and correctness of the file name\n");
        printf("Use name for files without spaces.\n");
    }
}
void ChangeDirectory(char* path)
{
#ifdef _WIN32
    if (!SetCurrentDirectoryA(path))
    {
        fprintf(stderr, "SetCurrentDirectoryA failed. err=%lu\n", GetLastError());
        return;
    }
#else
    if (chdir(path) != 0)
    {
        perror("chdir");
        return;
    }
#endif
    return;

}
void MakeDirectory(const char* dir)
{
#ifdef _WIN32
    if (!CreateDirectoryA(dir, NULL))
    {
        fprintf(stderr, "CreateDirectoryA failed. err=%lu\n", GetLastError());
        return;
    }
#else
    if (mkdir(dir, 0755) != 0)
    {
        perror("mkdir");
        return;
    }
#endif
    printf("Created: %s\n", dir);
}
void PrintCurrentPath()
{
#ifdef _WIN32
    char buf[MAX_PATH];
    DWORD len = GetCurrentDirectoryA((DWORD)sizeof(buf), buf);
    if (len == 0)
    {
        fprintf(stderr, "GetCurrentDirectoryA failed: err=%lu\n", GetLastError());
        return;
    }
    if (len >= sizeof(buf))
    {
        fprintf(stderr, "Path is too long for buffer\n");
        return;
    }
    printf("%s\n", buf);
#else
    char buf[PATH_MAX];
    if (getcwd(buf, sizeof(buf)) == NULL)
    {
        perror("getcwd");
        return;
    }
    printf("%s\n", buf);
#endif
}

//The function’s only job is to get the path as a string.
//Printing is handled elsewhere by the "PrintCurrentPath" command.
//This separation keeps the code clean and avoids excessive use of malloc.
char* GetCurrentPath(void)
{
#ifdef _WIN32
    DWORD len = GetCurrentDirectoryA(0, NULL);
    if (len == 0) return NULL;

    char* path = malloc(len);
    if (!path) return NULL;

    if (GetCurrentDirectoryA(len, path) == 0)
    {
        free(path);
        return NULL;
    }  
    return path;
#else
    char* path = getcwd(NULL, 0);
    if (!path) return NULL;
    return path;
#endif
}
void Clear()
{
#ifdef _WIN32
    system(CLEAR_SCREEN);
#else
    puts(CLEAR_SCREEN);
#endif
}
void List()
{
#ifdef _WIN32	
    WIN32_FIND_DATA findData;
    HANDLE hFind;

    hFind = FindFirstFile("*", &findData);
    if (hFind == INVALID_HANDLE_VALUE)
    {
        printf("Ошибка открытия каталога\n");
        return;
    }

    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE); //change color

    do
    {
        if (strcmp(findData.cFileName, ".") == 0 || strcmp(findData.cFileName, "..") == 0)
            continue;

        long long size = ((long long)findData.nFileSizeHigh << 32) + findData.nFileSizeLow;

        if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
        {
            SetConsoleTextAttribute(hOut, FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_INTENSITY);
            printf("%-30s [D] %6ld B\n", findData.cFileName, size);
            SetConsoleTextAttribute(hOut, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
        }
        else
        {
            if (size < 1024)
                printf("%-30s [F] %6ld B\n", findData.cFileName, size);
            else if (size < 1024 * 1024)
                printf("%-30s [F] %6.1f KB\n", findData.cFileName, size / 1024.0);
            else
                printf("%-30s [F] %6.1f MB\n", findData.cFileName, size / (1024.0 * 1024.0));
        }
    } while (FindNextFile(hFind, &findData));

    FindClose(hFind);
#else
    DIR *dir = opendir(".");
    if (!dir) return;

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL)
    {
        if (!strcmp(entry->d_name, ".") || !strcmp(entry->d_name, ".."))
            continue;

        struct stat st;
        if (stat(entry->d_name, &st) != 0)
            continue;
        long long size = (long long)st.st_size;

        if (stat(entry->d_name, &st) == 0 && S_ISDIR(st.st_mode))
            printf("\x1b[36m%-30s [D] %6lld B\x1b[0m\n", entry->d_name, size); // cyan
        else
        {
            if(size < 1024)
                printf("%-30s [F] %6lld B\n", entry->d_name, size);
            else if(size < 1024 * 1024)
                printf("%-30s [F] %6.1f KB\n", entry->d_name, size / 1024.0);
            else
                printf("%-30s [F] %6.1f MB\n", entry->d_name, size / (1024.0 * 1024.0));
        }

    }
    closedir(dir);
#endif
}
void Sv_Info()
{
	printf("P.V 0.01.0\n");
	printf("Information about sth\n");
}
int Sv_Sh()
{
    return 0;
}
void print_logo() 
{
    printf("  ____  __     __ \n");
    printf(" / ___| \\ \\   / / \n");
    printf(" \\___ \\  \\ \\ / /  \n");
    printf("  ___)|   \\ V /   \n");
    printf(" |____/    \\_/    \n\n");
    printf("use <help> to get information\n");
}
void Help()
{
  printf("\nusage: sv [-sh] - command nothing to do\n");
  printf("          [-info] - information about project\n\n\n");
  printf("main possibilities:\n");
  printf("\t<ls>\tdisplay directory contents\n");
  printf("\t<cl>\tclear the console\n");
  printf("\t<dir>\tcreate directory\n");
  printf("\t<cwd>\tpath to the current directory\n");
  printf("\t<cd>\tmove to another directory\n");
  printf("\t<cd ..>\tgo up one directory \n");
  printf("\t<cat>\tdisplay file contents in console\n");
  printf("\t<cf>\tcreate files\n");
  printf("\t<rm>\tdelete files\n");
  printf("\t<bin>\tviewing binary files\n");
  printf("\t<s>\tcheck size of file\n");
  printf("\t<tree>\trecursively traverses all subdirectories\n");
  printf("\t<exit>\tjust exit\n");
}