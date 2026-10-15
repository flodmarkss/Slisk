# Xbox 360 Wireless → PS4: utvecklingsstatus

Mål: USB-mottagare VID 045E:0719 direkt i PS4 med GoldHEN, DualShock 4 som spelare 1 och Xbox 360 som spelare 2. Ingen Windows-dator, Raspberry Pi eller Control4Free.

## Hårdvaruverifierat
- PS4 läser Xbox-mottagaren och knapptryckningarna.
- Xbox A/B/X/Y mappas till PS4 ✕/○/□/△.
- Styrspaksförändringar har registrerats.
- Fungerande read-hook-fria versioner har behållit DualShock 4.

## Allvarliga fel att inte återinföra
- Gamla scePadRead-hookar orsakade krascher eller konsollåsning.
- v0.7.5 kraschade utan logg.
- v0.7.9 låste konsolen; koden ignorerade resultat från scePadVirtualDeviceAddDevice och kunde binda fel MBus-enhet.
- v0.8.0 är offline-säkerhetsfixar, INTE en spelbar konsolrelease.

## Krävs inför nästa spelare-2-test
1. Korrekt PS4-systemkontext för virtuell kontrollregistrering.
2. Bekräftad API/ABI och identitet för virtuell enhet.
3. Komplett USB→Xbox-avkodning→PS4-mappning→virtuell enhet→PS4-användare→spelare 2.
4. Neutralisering av knappar, avregistrering, timeout, felinjektion och offline-tester.
5. PS4-korskompilering och därefter försiktigt konsoltest.

**Spelbar version är inte verifierad.** Offline-tester räcker inte för att intyga kompatibilitet med PS4.
