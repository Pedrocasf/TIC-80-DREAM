// MIT License

// Copyright (c) 2017 Vadim Grigoruk @nesbox // grigoruk@gmail.com

// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:

// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include "net.h"
#include "defines.h"
#include "version.h"

#include <psp2/net/net.h>
#include <psp2/net/netctl.h>
#include <psp2/net/http.h>
#include <psp2/sysmodule.h>

#include <SDL.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define URL_SIZE 2048
#define NET_POOL_SIZE (512 * 1024)
#define HTTP_POOL_SIZE (512 * 1024)
#define READ_PAGE_SIZE (16 * 1024)
#define TIMEOUT_USEC (10 * 1000 * 1000)

#define USER_AGENT "tic80-vita/" DEF2STR(TIC_VERSION_MAJOR) "." DEF2STR(TIC_VERSION_MINOR)

struct tic_net
{
    // held by the studio for the whole of its tick, so the worker threads below
    // can only ever reach a callback in between two of them
    SDL_mutex* tick;

    const char* host;
    s32 tmpl;
};

typedef struct
{
    char url[URL_SIZE];

    tic_net* net;
    net_get_data data;
    net_get_callback callback;

    void* buffer;
    s32 size;
} net_ctx;

static void report(net_ctx* ctx)
{
    if(!ctx->callback)
        return;

    SDL_LockMutex(ctx->net->tick);
    ctx->callback(&ctx->data);
    SDL_UnlockMutex(ctx->net->tick);
}

static void fail(net_ctx* ctx, s32 code)
{
    ctx->data.type = net_get_error;
    ctx->data.error.code = code;
    report(ctx);
}

// reads one open request to the end, reporting as it goes
static void transfer(net_ctx* ctx, s32 req)
{
    s32 res = sceHttpSendRequest(req, NULL, 0);
    if(res < 0)
    {
        fail(ctx, res);
        return;
    }

    s32 status = 0;
    if(sceHttpGetStatusCode(req, &status) < 0 || status != 200)
    {
        fail(ctx, status ? status : -1);
        return;
    }

    // a missing or chunked length is fine, it only feeds the progress bar
    unsigned long long length = 0;
    sceHttpGetResponseContentLength(req, &length);
    ctx->data.progress.total = (s32)length;

    for(;;)
    {
        u8* grown = realloc(ctx->buffer, ctx->size + READ_PAGE_SIZE);
        if(!grown)
        {
            fail(ctx, -1);
            return;
        }

        ctx->buffer = grown;

        s32 read = sceHttpReadData(req, grown + ctx->size, READ_PAGE_SIZE);
        if(read < 0)
        {
            fail(ctx, read);
            return;
        }

        if(read == 0)
            break;

        ctx->size += read;

        ctx->data.type = net_get_progress;
        ctx->data.progress.size = ctx->size;

        if(ctx->data.progress.total < ctx->size)
            ctx->data.progress.total = ctx->size;

        report(ctx);
    }

    ctx->data.type = net_get_done;
    ctx->data.done.data = ctx->buffer;
    ctx->data.done.size = ctx->size;
    report(ctx);
}

// the whole of one request, start to finish, on its own thread
static void execute(net_ctx* ctx)
{
    ctx->data.url = ctx->url;

    s32 conn = sceHttpCreateConnectionWithURL(ctx->net->tmpl, ctx->url, SCE_FALSE);
    if(conn < 0)
    {
        fail(ctx, conn);
        return;
    }

    s32 req = sceHttpCreateRequestWithURL(conn, SCE_HTTP_METHOD_GET, ctx->url, 0);

    if(req < 0)
        fail(ctx, req);
    else
    {
        transfer(ctx, req);
        sceHttpDeleteRequest(req);
    }

    sceHttpDeleteConnection(conn);
}

static int thread(void* data)
{
    net_ctx* ctx = data;

    execute(ctx);

    FREE(ctx->buffer);
    free(ctx);

    return 0;
}

tic_net* tic_net_create(const char* host)
{
    // no point starting any of this up without a network to reach
    if(sceSysmoduleLoadModule(SCE_SYSMODULE_NET) < 0)
        return NULL;

    static SceNetInitParam param;
    param.memory = malloc(NET_POOL_SIZE);
    param.size = NET_POOL_SIZE;
    param.flags = 0;

    if(!param.memory)
        return NULL;

    // already up is as good as brought up, the return is only fatal otherwise
    s32 res = sceNetInit(&param);
    if(res < 0 && res != (s32)SCE_NET_ERROR_EBUSY)
    {
        free(param.memory);
        return NULL;
    }

    sceNetCtlInit();

    s32 state = 0;
    if(sceNetCtlInetGetState(&state) < 0 || state != SCE_NETCTL_STATE_CONNECTED)
        return NULL;

    if(sceSysmoduleLoadModule(SCE_SYSMODULE_HTTP) < 0)
        return NULL;

    if(sceHttpInit(HTTP_POOL_SIZE) < 0)
        return NULL;

    s32 tmpl = sceHttpCreateTemplate(USER_AGENT, SCE_HTTP_VERSION_1_1, SCE_TRUE);
    if(tmpl < 0)
    {
        sceHttpTerm();
        return NULL;
    }

    sceHttpSetResolveTimeOut(tmpl, TIMEOUT_USEC);
    sceHttpSetConnectTimeOut(tmpl, TIMEOUT_USEC);
    sceHttpSetSendTimeOut(tmpl, TIMEOUT_USEC);
    sceHttpSetRecvTimeOut(tmpl, TIMEOUT_USEC);
    sceHttpSetAutoRedirect(tmpl, SCE_TRUE);

    tic_net* net = NEW(tic_net);

    *net = (tic_net)
    {
        .tick = SDL_CreateMutex(),
        .host = host,
        .tmpl = tmpl,
    };

    // the studio hands us the site with its scheme on the front, and this
    // platform talks to it over plain http, see build/vita/README.md
    static const char Https[] = "https://";
    if(strstr(host, Https) == host)
        net->host += STRLEN(Https);

    static const char Http[] = "http://";
    if(strstr(host, Http) == host)
        net->host += STRLEN(Http);

    return net;
}

void tic_net_get(tic_net* net, const char* url, net_get_callback callback, void* calldata)
{
    net_get_data data = {.calldata = calldata, .url = url};

    // no network, no browsing: answer right away so the console and SURF do not
    // sit waiting on something that is never coming
    if(!net)
    {
        if(callback)
        {
            data.type = net_get_error;
            data.error.code = -1;
            callback(&data);
        }

        return;
    }

    net_ctx* ctx = NEW(net_ctx);

    *ctx = (net_ctx)
    {
        .net = net,
        .callback = callback,
        .data = data,
    };

    snprintf(ctx->url, sizeof ctx->url, "http://%s%s", net->host, url);

    SDL_Thread* worker = SDL_CreateThread(thread, "tic80 net", ctx);

    if(worker)
        SDL_DetachThread(worker);
    else
    {
        free(ctx);

        if(callback)
        {
            data.type = net_get_error;
            data.error.code = -1;
            callback(&data);
        }
    }
}

void tic_net_close(tic_net* net)
{
    if(!net)
        return;

    sceHttpDeleteTemplate(net->tmpl);
    sceHttpTerm();
    SDL_DestroyMutex(net->tick);
    free(net);
}

void tic_net_start(tic_net* net)
{
    if(net)
        SDL_LockMutex(net->tick);
}

void tic_net_end(tic_net* net)
{
    if(net)
        SDL_UnlockMutex(net->tick);
}
