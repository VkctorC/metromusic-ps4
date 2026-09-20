#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <orbis/libkernel.h>

#define LOG_PATH "/data/metro_music_probe.log"

typedef int (*NotifyFn)(int, char *);

static FILE *g_log = NULL;

static void log_line(const char *label, long long value) {
    if (!g_log) return;
    fprintf(g_log, "%s: 0x%llx (%lld)\n", label, (unsigned long long)value, value);
    fflush(g_log);
}

static void log_symbol(uint32_t handle, const char *name, int *ok_count) {
    void *addr = NULL;
    int32_t ret = sceKernelDlsym((int32_t)handle, name, &addr);
    if (g_log) {
        fprintf(g_log, "dlsym %-52s ret=0x%08x addr=%p\n", name, (uint32_t)ret, addr);
        fflush(g_log);
    }
    if (ret == 0 && addr != NULL && ok_count) (*ok_count)++;
}

static NotifyFn get_notify(void) {
    int res = 0;
    uint32_t h = sceKernelLoadStartModule("/system/common/lib/libSceSysUtil.sprx", 0, NULL, 0, NULL, &res);
    if ((int32_t)h < 0) return NULL;
    void *addr = NULL;
    if (sceKernelDlsym((int32_t)h, "sceSysUtilSendSystemNotificationWithText", &addr) != 0)
        return NULL;
    return (NotifyFn)addr;
}

int main(void) {
    g_log = fopen(LOG_PATH, "w");
    if (g_log) {
        fprintf(g_log, "MetroMusic CustomMusicCore probe v0.1\n");
        fprintf(g_log, "Firmware target: PS4 13.52 (runtime probe)\n\n");
        fflush(g_log);
    }

    NotifyFn notify = get_notify();
    if (notify) notify(222, "MetroMusic Probe: iniciando CustomMusicCore");



    int module_res = 0;
    uint32_t core = sceKernelLoadStartModule(
        "/system/common/lib/libSceCustomMusicCore.sprx",
        0, NULL, 0, NULL, &module_res);
    log_line("CustomMusicCore handle", (int32_t)core);
    log_line("CustomMusicCore load result", module_res);

    int service_res = 0;
    uint32_t service = sceKernelLoadStartModule(
        "/system/common/lib/libSceCustomMusicService.sprx",
        0, NULL, 0, NULL, &service_res);
    log_line("CustomMusicService handle", (int32_t)service);
    log_line("CustomMusicService load result", service_res);

    int ok = 0;
    if ((int32_t)core >= 0) {
        static const char *symbols[] = {
            "sceCustomMusicCoreBgmOutput",
            "sceCustomMusicCoreBgmStop",
            "sceCustomMusicCoreGetBgmAuthorityStatus",
            "sceCustomMusicCoreGetSystemAudioVolume",
            "sceCustomMusicCoreImposeSetPlayStatusInfo2",
            "sceCustomMusicCoreImposeSetSpTrackInfo",
            "sceCustomMusicCoreRegisterAppExFunctionTable",
            "sceCustomMusicCoreRegisterSpImposeFunctionTable",
            "sceCustomMusicCoreSendEvent",
            "sceCustomMusicCoreSendMulticastEvent",

            /* Semantic names seen in Spotify strings. These may have
               different exported names/NIDs on retail firmware. */
            "sceCustomMusicCoreNotifyPlaybackHasStarted",
            "sceCustomMusicCoreRegisterImposeFunctionTable",
            "sceCustomMusicCoreSendSpErrorMessage",
            "sceCustomMusicCoreStartToAcceptUserOperation"
        };
        size_t count = sizeof(symbols) / sizeof(symbols[0]);
        if (g_log) fprintf(g_log, "\n--- symbol probe ---\n");
        for (size_t i = 0; i < count; ++i)
            log_symbol(core, symbols[i], &ok);
    }

    if (g_log) {
        fprintf(g_log, "\nResolved named symbols: %d\n", ok);
        fprintf(g_log, "Probe finished. No CustomMusicCore function was invoked.\n");
        fflush(g_log);
    }

    if (notify) {
        char msg[160];
        snprintf(msg, sizeof(msg), "MetroMusic Probe: %d simbolos resolvidos. Log em /data/metro_music_probe.log", ok);
        notify(222, msg);
    }

    if (g_log) fclose(g_log);

    /* Keep the app alive briefly so the notification/log can be observed. */
    sleep(8);
    return 0;
}
