# Slisk v0.9.0 – fristående Xbox 360 → PS4 GoldHEN systempayload (EXPERIMENTELL)

**Mål:** Xbox 360 Wireless Receiver VID 045E / PID 0719 i PS4 USB, utan Windows/PC, Raspberry Pi eller installerad Control4Free. DualShock 4 för spelare 1 och Xbox 360 som virtuell DualShock 4 för spelare 2.

**Status:** Kod och hosttester under utveckling. Detta är INTE konsolverifierat eller garanterat spelbart. Kör INTE äldre PRX-filer v0.7.5 eller v0.7.9 som låste konsolen. En systempayload behöver korrekt GoldHEN-version och rättigheter.

## Arkitektur

- Egen systempayload (ELF), **inte** spel-PRX. Startas via GoldHEN PayLoader.
- Återanvänder v0.7.0:s verkligt PS4-testade trådlösa Xbox-paketavkodning.
- Sony libSceUsbd via `dlopen`/`dlsym`; inspekterar USB-interface/endpoint-deskriptorer. Ingen nätverkstransport.
- Använder PS4-VDA-kod byggd från en **pinnad upstream GPL-3.0-källa** (Control4Free), men kräver inte att programmet är installerat på PS4.
- Startar inte AddDevice förrän fysisk Xbox A registrerats, `/dev/klog` har öppnats och ett unikt klog-testmeddelande har mottagits.
- Kräver virtuell `DEVICE_ADDED` subtype 2 och giltig separat `DEVICE_OWNER_CHANGED`, samt kontroll att användaren inte är DualShock 4:s tidigare profil.
- Neutraliserar indata, stoppar och försöker ta bort den virtuella enheten vid fel, frånkoppling eller tidsgräns.
- Högst **en** trådlös Xbox-kontroll i första testet.

## Uppstart / loggar (när en granskad ELF faktiskt finns)

En kompatibel GoldHEN-installation med PayLoader (minst v2.4b18.10 enligt referensprojektet) krävs för systempayloads. Starta inte från `plugins.ini`; denna ELF tillhör GoldHEN-payloadservern. När användaren ska testköra den kommer instruktioner för rätt loader och säker avstängning att bifogas.

Loggar: `/data/slisk_x360/control4free.log` (historiskt filnamn från VDA-komponenten, inte ett runtime-beroende på Control4Free).

## Licens

`src/xw_core.*` kommer från det tidigare Slisk-utvecklingsarbetet. Virtuellt PS4-stöd vid bygge hämtas från GPL-3.0-projektet Control4Free av MoHadiShibli, vars licens måste bibehållas för den sammanlänkade ELF:en. `src/main.c` och `src/slisk_usb.c` är nya Slisk-integrationsmoduler licensierade GPL-3.0-only i denna variant. Vidare distribution ska ha fullständig GPL-3.0-källkod eller motsvarande erbjudande.

## Hårdvarurisk

Tidigare v0.7.9 hängde hela användarens konsol. Offline-tester och cross-build är nödvändiga men **inte tillräckliga** för att garantera att den nya payloaden är säker. Följ uttrycklig installationsinstruktion först när nästa test är berett.
