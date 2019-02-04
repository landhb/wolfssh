/* sftp.c
 *
 * Copyright (C) 2014-2016 wolfSSL Inc.
 *
 * This file is part of wolfSSH.
 *
 * wolfSSH is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * wolfSSH is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with wolfSSH.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include <wolfssh/ssh.h>
#include <wolfssh/wolfsftp.h>

#define WOLFSSH_TEST_LOCKING
#define WOLFSSH_TEST_THREADING
#include <wolfssh/test.h>

#include "tests/testsuite.h"
#include "examples/echoserver/echoserver.h"
#include "examples/sftpclient/sftpclient.h"

#if defined(WOLFSSH_SFTP) && !defined(SINGLE_THREADED)

static const char* cmds[] = {
    "ls",
    "exit"
};
static int commandIdx = 0;

static int commandCb(const char* in, char* out, int outSz)
{
    int ret = 0;

    if (in) {
        /* print out */
    }

    /* get command input */
    if (out) {
        int sz = WSTRLEN(cmds[commandIdx]);
        if (outSz < sz) {
            ret = -1;
        }
        else {
            WMEMCPY(out, cmds[commandIdx], sz);
        }
        commandIdx++;
    }
    return ret;
}


/* test SFTP commands, if flag is set to 1 then use non blocking
 * return 0 on success */
int test_SFTP(int flag)
{
    func_args ser;
    func_args cli;
    tcp_ready ready;
    int ret = 0;
    int argsCount;

    const char* args[10];
    char  portNumber[8];

    THREAD_TYPE serThread;
    THREAD_TYPE cliThread;

    WMEMSET(&ser, 0, sizeof(func_args));
    WMEMSET(&cli, 0, sizeof(func_args));
    commandIdx = 0;

    argsCount = 0;
    args[argsCount++] = ".";
    args[argsCount++] = "-1";
#ifndef USE_WINDOWS_API
    args[argsCount++] = "-p";
    args[argsCount++] = "0";
#endif
    if (flag)
        args[argsCount++] = "-N";

    ser.argv   = (char**)args;
    ser.argc    = argsCount;
    ser.signal = &ready;
    InitTcpReady(ser.signal);
    ThreadStart(echoserver_test, (void*)&ser, &serThread);
    WaitTcpReady(&ser);

    argsCount = 0;
    args[argsCount++] = ".";
    args[argsCount++] = "-u";
    args[argsCount++] = "jill";
    args[argsCount++] = "-P";
    args[argsCount++] = "upthehill";
    args[argsCount++] = "-p";

#ifndef USE_WINDOWS_API
    /* use port that server has found */
    snprintf(portNumber, sizeof(portNumber), "%d", ready.port);
    args[argsCount++] = portNumber;
#endif

    if (flag)
        args[argsCount++] = "-N";

    cli.argv    = (char**)args;
    cli.argc    = argsCount;
    cli.signal  = &ready;
    cli.sftp_cb = commandCb;
    ThreadStart(sftpclient_test, (void*)&cli, &cliThread);


    ThreadJoin(serThread);
    ThreadJoin(cliThread);

    return ret;
}
#endif /* WOLFSSH_SFTP */


