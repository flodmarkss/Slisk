# Installationsspärr innan PS4-test

Använd INTE källkodspaketet som en spelplugin. ELF måste först byggas via
systempayload/.github/workflows/slisk-build.yml och kontrolleras mot PS4 SDK.

Den tidigare v0.7.9 låste konsolen när virtuella Pad-anrop gjordes från
spelprocessen. Den här versionen gör anrop från en GoldHEN-systempayload,
men säkerheten och kompatibiliteten är ännu inte verifierade på fysisk PS4.

Nästa steg för ett försiktigt konsoltest när CI-byggningen är grön:

- Kontrollera firmware och GoldHEN-version, minst 2.4b18.10.
- Inga gamla Xbox-PRX i plugins.ini och ingen separat Control4Free.
- Starta systempayloadens ELF via GoldHEN PayLoader, inte plugins.ini.
- Slå på Xbox 360-mottagaren som sitter direkt i PS4.
- Tryck A endast när loggen visar att USB-läsningen är igång.
- Välj en annan användare/gäst när PS4 visar användarvalet.
- Om något hänger sig, upprepa inte testet och avaktivera payloadens AutoRun.

Följ aktuella slutliga instruktioner som ges separat efter grön CI-byggning.
