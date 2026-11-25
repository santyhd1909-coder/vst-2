# DALUmagic (VST3)

Sintetizador estilo R&B 2000s / plugg / rage. Frutiger Metro rojo, negro y blanco.
174 presets, osciladores con unison (supersaw), modo legato con slide, filtro con formantes vocales, y 5 efectos.

## Forma facil: que GitHub lo compile por vos (sin instalar nada)

1. Crea una cuenta gratis en https://github.com y un repositorio nuevo llamado `DALUmagic` (privado esta bien).
2. Sube TODO el contenido de esta carpeta al repositorio (boton "Add file" > "Upload files").
   Tiene que quedar la carpeta oculta `.github/workflows/build.yml` en el repo.
   Si el navegador no la sube, crea el archivo a mano: "Add file" > "Create new file", escribi
   `.github/workflows/build.yml` como nombre y pega el contenido del archivo.
3. Entra a la pestana **Actions**. El workflow "Build DALUmagic VST3" arranca solo al subir los archivos
   (o tocas "Run workflow"). La primera vez tarda 10 a 15 minutos.
4. Cuando tenga el tilde verde, abri esa ejecucion y baja **DALUmagic-VST3-Windows** (seccion Artifacts).
5. Descomprimi el zip: adentro hay una carpeta `DALUmagic.vst3`.
6. Copia la carpeta `DALUmagic.vst3` completa a: `C:\Program Files\Common Files\VST3\`
7. Abri FL Studio > **Options > Manage plugins** > **Find installed plugins** (con "Verify plugins" tildado).
8. Agrega DALUmagic en el Channel Rack (+ > More plugins), o desde el Plugin database > Installed > Generators.

## Forma local (Windows)

Necesitas: Git, CMake y Visual Studio 2022 Community con "Desktop development with C++".

    cmake -B build -G "Visual Studio 17 2022" -A x64
    cmake --build build --config Release --target DALUmagic_VST3

Resultado: `build/DALUmagic_artefacts/Release/VST3/DALUmagic.vst3` (copiarlo a la carpeta VST3 como en el paso 6).
Tambien se genera una app standalone (`--target DALUmagic_Standalone`) para probar sin FL.

## Uso

- **Presets**: elegi categoria a la izquierda y toca uno de la lista, o usa los botones < >.
- **Modo**: `Poly` o `Legato`. En legato las notas superpuestas se deslizan (slide); el tiempo se regula con **Glide**.
- **Unison**: 1 a 9 voces detuneadas (supersaw). **Detune** controla la apertura.
- **Efectos**: boton "Anadir efecto" abre el menu (Drive, Chorus, Tremolo, Delay, Reverb). La X de cada tarjeta lo quita.
- Las perillas se mueven arrastrando arriba/abajo; doble click vuelve al valor por defecto.
- El parlante reacciona al sonido que sale del plugin.

## Notas

- Formato: VST3 de 64 bits para Windows. Para macOS hay que compilar en una Mac con el mismo comando (Xcode) y firmar el plugin.
- `DALUmagic_web_version.html` es la version web, se abre directo en el navegador.
