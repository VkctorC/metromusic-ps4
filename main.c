#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <orbis/libkernel.h>

#define LOG_PATH "/data/metro_music_probe.log"
static int32_t g_log = -1;

static void log_text(const char *text)
{
    printf("%s", text);
    fflush(stdout);
    if (g_log >= 0)
        sceKernelWrite(g_log, text, strlen(text));
}

static void log_value(const char *name, int64_t value)
{
    char buf[256];
    snprintf(buf, sizeof(buf), "%s = 0x%llx (%lld)\n",
             name, (unsigned long long)value, (long long)value);
    log_text(buf);
}

static void probe_symbol(int32_t handle, const char *name)
{
    char buf[320];
    void *addr = NULL;
    snprintf(buf, sizeof(buf), "ANTES dlsym: %s\n", name);
    log_text(buf);
    int32_t ret = sceKernelDlsym(handle, name, &addr);
    snprintf(buf, sizeof(buf),
             "DEPOIS dlsym: %-48s ret=0x%08x addr=%p\n",
             name, (uint32_t)ret, addr);
    log_text(buf);
}

int main(void)
{
    setvbuf(stdout, NULL, _IONBF, 0);

    printf("\n========================================\n"
           " MetroMusic Probe v0.14 - BOOT\n"
           "========================================\n");

    g_log = sceKernelOpen(LOG_PATH, O_WRONLY | O_CREAT | O_TRUNC, 0777);

    log_text("\n========================================\n"
             " MetroMusic CustomMusicCore Probe v0.14\n"
             " PS4 firmware alvo: 13.52\n"
             "========================================\n\n");

    log_value("Log file descriptor", g_log);
    if (g_log < 0)
        log_text("AVISO: log em /data indisponivel; continuando pelo Klog.\n\n");
    else
        log_text("Arquivo /data/metro_music_probe.log aberto.\n\n");

    log_text("STAGE 0: main() iniciado normalmente\n");
    log_text("\nSTAGE 1: antes de carregar libSceCustomMusicCore.sprx\n");

    int32_t core_result = 0;
    int32_t core = sceKernelLoadStartModule(
        "/system/common/lib/libSceCustomMusicCore.sprx",
        0, NULL, 0, NULL, &core_result);

    log_text("STAGE 2: voltou de sceKernelLoadStartModule(CustomMusicCore)\n");
    log_value("CustomMusicCore handle", core);
    log_value("CustomMusicCore result", core_result);

    if (core >= 0) {
        log_text("\n========================================\n"
                 " TESTANDO EXPORTS CustomMusicCore\n"
                 "========================================\n");
        probe_symbol(core, "sceCustomMusicCoreBgmOutput");
        probe_symbol(core, "sceCustomMusicCoreBgmStop");
        probe_symbol(core, "sceCustomMusicCoreGetBgmAuthorityStatus");
        probe_symbol(core, "sceCustomMusicCoreGetSystemAudioVolume");
        probe_symbol(core, "sceCustomMusicCoreImposeSetPlayStatusInfo2");
        probe_symbol(core, "sceCustomMusicCoreImposeSetSpTrackInfo");
        probe_symbol(core, "sceCustomMusicCoreRegisterAppExFunctionTable");
        probe_symbol(core, "sceCustomMusicCoreRegisterSpImposeFunctionTable");
        probe_symbol(core, "sceCustomMusicCoreRegisterImposeFunctionTable");
        probe_symbol(core, "sceCustomMusicCoreStartToAcceptUserOperation");
        probe_symbol(core, "sceCustomMusicCoreNotifyPlaybackHasStarted");
        probe_symbol(core, "sceCustomMusicCoreSendEvent");
        probe_symbol(core, "sceCustomMusicCoreSendMulticastEvent");
    } else {
        log_text("\nCustomMusicCore nao foi carregado; pulando exports.\n");
    }

    log_text("\nSTAGE 3: antes de carregar libSceCustomMusicService.sprx\n");

    int32_t service_result = 0;
    int32_t service = sceKernelLoadStartModule(
        "/system/common/lib/libSceCustomMusicService.sprx",
        0, NULL, 0, NULL, &service_result);

    log_text("STAGE 4: voltou de sceKernelLoadStartModule(CustomMusicService)\n");
    log_value("CustomMusicService handle", service);
    log_value("CustomMusicService result", service_result);

    log_text("\n========================================\n"
             "STAGE 5: probe terminou normalmente\n"
             "Nenhuma funcao CustomMusicCore foi executada; apenas load + dlsym.\n"
             "========================================\n\n"
             "Mantendo aplicativo vivo por 30 segundos...\n");

    sleep(30);
    log_text("MetroMusic Probe encerrando normalmente.\n");
    if (g_log >= 0) sceKernelClose(g_log);
    return 0;
}
