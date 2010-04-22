#include <cerrno>
#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>

#include "UnixEnvironmentSetter.hxx"

static const char * ENV_VAR_NAME = "SAL_DISABLE_GRAPHITE";
static const char * ORIG_HOME = getenv("HOME");
static const char * SHELL = "/bin/sh";

static int runTest(int num, const char * fileData, const char * value, bool replace)
{
    static const char * testfile = "temphome";
    char filepath[1024];
    char homepath[1024];
    char homeenv[1024];
    char shTest[1024];
    char * env[2];
    snprintf(homepath, 1024, "%s/%s%02d", ORIG_HOME, testfile, num);
    snprintf(homeenv, 1024, "HOME=%s", homepath);
    snprintf(filepath, 1024, "%s/.profile", homepath);
    // delete if it exists
    unlink(filepath);
    rmdir(homepath);
    int status = mkdir(homepath, 0700);
    assert(status == 0 || errno == EEXIST);
   FILE * f = NULL;
   if (fileData)
   {
        // write existing data
        f = fopen(filepath, "w");
        fwrite(fileData, 1, strlen(fileData), f);
        fclose(f);
   }
   if (org::sil::graphite::UnixEnvironmentSetter::parseFile(filepath, ENV_VAR_NAME, value) != true)
   {
       fprintf(stderr, "Failed to parseFile %s\n", filepath);
       return 1;
   }
   snprintf(shTest, 1024, "test x$%s = x%s; ", ENV_VAR_NAME, value);
   status = 0;
   int pid = fork();
   if (pid)
   {
       waitpid(pid, &status, 0);
   }
   else
   {
       //setenv("HOME", homepath, 1);
       env[0] = homeenv;
       env[1] = NULL;
       execle(SHELL, SHELL, "-i","-l", "-c", shTest, NULL, env);
       //execle("/bin/pwd", "/bin/pwd", NULL, env);
   }
   if (!WIFEXITED(status))
   {
       fprintf(stderr, "Failed to run shell. Status %d for test %d\n", status, num);
       return -1;
   }
   status = WEXITSTATUS(status);
   if (status)
   {
       fprintf(stderr, "%s -c -l %s\nUnexpected result %d for test #%d\n", SHELL, shTest, status, num);
   }
   else
   {
       // check file size
       struct stat filestat;
       if (replace)
       {
           stat(filepath, &filestat);
           if (filestat.st_size != static_cast<int>(strlen(fileData)))
           {
               fprintf(stderr, "Environment variable was not modified in place for test %d expected %d, got %ld\n",
                       num, strlen(fileData)+1, filestat.st_size);
               return -2;
           }
       }
       unlink(filepath); // delete file
       rmdir(homepath);
   }

   return status;
}

int testShellTest()
{
    int testStatus = 0;
    int status = 0;
    const char * env[2];
    env[0] = "A=1";
    env[1] = NULL;
    int pid = fork();
    if (pid)
   {
       waitpid(pid, &status, 0);
   }
   else
   {
       execle(SHELL, "-i","-l", "-c", "test x$A = x1", NULL, env);
   }
   assert(WIFEXITED(status));
   status = WEXITSTATUS(status);
   if (status)
   {
       testStatus = 1;
   }
   else
   {
       pid = fork();
       if (pid)
        {
            waitpid(pid, &status, 0);
        }
        else
        {
            execle(SHELL, "-i","-l", "-c", "test x$A = x0", NULL, env);
        }
        assert(WIFEXITED(status));
        status = WEXITSTATUS(status);
        if (!status)
        {
            testStatus = 1;
        }
   }
   return testStatus;
}

int main(int argc, char ** argv)
{
    int status = 0;
    int testNum = 0;
    status = testShellTest();
    assert(status == 0);
    // test non existant
    status |= runTest(++testNum, NULL, "0", false);
    status |= runTest(++testNum, NULL, "1", false);
// test on empty file
    status |= runTest(++testNum, "", "0", false);
    status |= runTest(++testNum, "", "1", false);
    // test with existing value
    status |= runTest(++testNum, "export SAL_DISABLE_GRAPHITE=0\n", "0", true);
    status |= runTest(++testNum, "export SAL_DISABLE_GRAPHITE=0\n", "1", true);
    status |= runTest(++testNum, "export SAL_DISABLE_GRAPHITE=1\n", "0", true);
    status |= runTest(++testNum, "export SAL_DISABLE_GRAPHITE=1\n", "1", true);

    // test with duplicates
    status |= runTest(++testNum, "export SAL_DISABLE_GRAPHITE=0\nexport SAL_DISABLE_GRAPHITE=1\n", "0", true);
    status |= runTest(++testNum, "export SAL_DISABLE_GRAPHITE=0\nexport SAL_DISABLE_GRAPHITE=1\n", "1", true);
    status |= runTest(++testNum, "export SAL_DISABLE_GRAPHITE=1\nexport SAL_DISABLE_GRAPHITE=0\n", "0", true);
    status |= runTest(++testNum, "export SAL_DISABLE_GRAPHITE=1\nexport SAL_DISABLE_GRAPHITE=0\n", "1", true);

    // Multiple lines
    status |= runTest(++testNum, "# example\nexport SAL_DISABLE_GRAPHITE=0\n#end\n", "0", true);
    status |= runTest(++testNum, "# example\nexport SAL_DISABLE_GRAPHITE=0\n#end\n", "1", true);
    status |= runTest(++testNum, "# example\nexport SAL_DISABLE_GRAPHITE=1\n#end\n", "0", true);
    status |= runTest(++testNum, "# example\nexport SAL_DISABLE_GRAPHITE=1\n#end\n", "1", true);

    // commmented
    status |= runTest(++testNum, "#export SAL_DISABLE_GRAPHITE=0\n", "0", false);
    status |= runTest(++testNum, "#export SAL_DISABLE_GRAPHITE=0\n", "1", false);
    status |= runTest(++testNum, "# export SAL_DISABLE_GRAPHITE=1\n", "0", false);
    status |= runTest(++testNum, "# export SAL_DISABLE_GRAPHITE=1\n", "1", false);

    // padding
    status |= runTest(++testNum, " export SAL_DISABLE_GRAPHITE=0 # for ooo\n", "0", true);
    status |= runTest(++testNum, " export SAL_DISABLE_GRAPHITE=0 # for ooo\n", "1", true);
    status |= runTest(++testNum, " export SAL_DISABLE_GRAPHITE=1 # for ooo\n", "0", true);
    status |= runTest(++testNum, " export SAL_DISABLE_GRAPHITE=1 # for ooo\n", "1", true);

    // test the buffer wrapping code
    char fileData[2048];
    int i = 0;
    for (; i < 1500; )
    {
        fileData[i++] = '#';
        fileData[i++] = '\n';
    }
    fileData[i] = '\0';
    status |= runTest(++testNum, fileData, "0", false);
    status |= runTest(++testNum, fileData, "1", false);
    strncpy(fileData + i, "export SAL_DISABLE_GRAPHITE=1\0", 30);
    status |= runTest(++testNum, fileData, "0", true);
    status |= runTest(++testNum, fileData, "1", true);
    strncpy(fileData + i, "export SAL_DISABLE_GRAPHITE=0\0", 30);
    status |= runTest(++testNum, fileData, "0", true);
    status |= runTest(++testNum, fileData, "1", true);

    strncpy(fileData + i, "export SAL_DISABLE_GRAPHITE=1\n\0", 31);
    status |= runTest(++testNum, fileData, "0", true);
    status |= runTest(++testNum, fileData, "1", true);
    strncpy(fileData + i, "export SAL_DISABLE_GRAPHITE=0\n\0", 31);
    status |= runTest(++testNum, fileData, "0", true);
    status |= runTest(++testNum, fileData, "1", true);
    return status;
}
