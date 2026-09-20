# MetroMusic CustomMusicCore Probe v0.1

PoC para PS4 que testa se um FPKG/homebrew comum consegue carregar o sysmodule CustomMusicCore e resolver simbolos da biblioteca de musica do sistema.

Ele **nao chama** as funcoes CustomMusicCore ainda. O objetivo desta versao e reduzir risco enquanto confirmamos acesso no firmware 13.52.

## Build automatico no GitHub

O workflow `.github/workflows/build.yml` usa a action oficial do OpenOrbis com toolchain v0.5.4.

1. Envie estes arquivos para um repositorio GitHub.
2. Abra a aba **Actions**.
3. Escolha **Build MetroMusic PS4 Probe**.
4. Toque em **Run workflow** se nenhum build estiver rodando.
5. Quando ficar verde, abra a execucao e baixe o artifact **MetroMusic-CustomMusicCore-Probe-v0.1**.
6. Dentro dele estara o `.pkg` para instalar no PS4.

O OpenOrbis documenta `OpenOrbis/toolchain-action@main` como o metodo de GitHub Actions para compilar projetos com o toolchain.

## Teste no PS4

1. Ative GoldHEN/HEN.
2. Instale e abra o PKG.
3. Aguarde a notificacao do probe.
4. Via FTP, copie `/data/metro_music_probe.log`.
5. Envie o log para a proxima etapa da engenharia reversa.

## Saida esperada

`IV0000-BREW00991_00-METROMUSICPROBE0.pkg`
