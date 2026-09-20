#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>

#include <orbis/libkernel.h>

#define LOG_PATH "/data/metro_music_probe.log"

static int32_t g_log = -1;

static void log_text(const char *text)
{
    if (g_log < 0)
        return;

    sceKernelWrite(g_log, text, strlen(text));
}

static void log_value(const char *name, int64_t value)
{
    char buf[256];

    snprintf(
        buf,
        sizeof(buf),
        "%s = 0x%llx (%lld)\n",
        name,
        (unsigned long long)value,
        (long long)value
    );

    log_text(buf);
}

static void probe_symbol(int32_t handle, const char *name)
{
    char buf[256];
    void *addr = NULL;

    snprintf(buf, sizeof(buf),
             "ANTES dlsym: %s\n", name);
    log_text(buf);

    int32_t ret = sceKernelDlsym(handle, name, &addr);

    snprintf(
        buf,
        sizeof(buf),
        "DEPOIS dlsym: %-48s ret=0x%08x addr=%p\n",
        name,
        (uint32_t)ret,
        addr
    );

    log_text(buf);
}

int main(void)
{
    /*
     * GoldHEN usa sceKernelOpen para /data porque fopen()
     * pode não ter permissão nesse caminho.
     */
    g_log = sceKernelOpen(
        LOG_PATH,
        0x200 | 0x001,
        0777
    );

    if (g_log < 0)
    {
        /*
         * Se nem /data funcionar, o app fica vivo 20 s.
         * Assim distinguimos erro de escrita de crash no loader.
         */
        sleep(20);
        return 0;
    }

    log_text(
        "====================================\n"
        "MetroMusic CustomMusicCore Probe v0.11\n"
        "PS4 firmware alvo: 13.52\n"
        "====================================\n\n"
    );

    log_text("STAGE 0: main() iniciado\n");

    /* ===================================================== */
    /* CustomMusicCore                                       */
    /* ===================================================== */

    log_text(
        "STAGE 1: antes de carregar "
        "libSceCustomMusicCore.sprx\n"
    );

    int32_t core = sceKernelLoadStartModule(
        "/system/common/lib/libSceCustomMusicCore.sprx",
        0,
        NULL,
        0,
        NULL,
        NULL
    );

    log_text("STAGE 2: voltou do load CustomMusicCore\n");
    log_value("CustomMusicCore handle", core);

    if (core >= 0)
    {
        log_text(
            "\n--- TESTANDO EXPORTS CustomMusicCore ---\n"
        );

        probe_symbol(
            core,
            "sceCustomMusicCoreBgmStop"
        );

        probe_symbol(
            core,
            "sceCustomMusicCoreBgmOutput"
        );

        probe_symbol(
            core,
            "sceCustomMusicCoreGetBgmAuthorityStatus"
        );

        probe_symbol(
            core,
            "sceCustomMusicCoreImposeSetSpTrackInfo"
        );

        probe_symbol(
            core,
            "sceCustomMusicCoreStartToAcceptUserOperation"
        );
    }

    /* ===================================================== */
    /* CustomMusicService                                    */
    /* ===================================================== */

    log_text(
        "\nSTAGE 3: antes de carregar "
        "libSceCustomMusicService.sprx\n"
    );

    int32_t service = sceKernelLoadStartModule(
        "/system/common/lib/libSceCustomMusicService.sprx",
        0,
        NULL,
        0,
        NULL,
        NULL
    );

    log_text("STAGE 4: voltou do load CustomMusicService\n");
    log_value("CustomMusicService handle", service);

    log_text(
        "\nSTAGE 5: probe terminou normalmente\n"
        "Mantendo processo vivo por 20 segundos...\n"
    );

    sleep(20);

    log_text("Encerrando probe.\n");

    sceKernelClose(g_log);

    return 0;
}
