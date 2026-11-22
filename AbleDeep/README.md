# AbleDeep – Deep House, Soulful House & Detroit Techno

AbleDeep ist ein Software-Instrument (VST3 / AU) für Ableton Live und andere DAWs. Es ist auf den
warmen, jazzigen Sound von Deep House und Soulful House zugeschnitten, wie man ihn von Kerri
Chandler, Atjazz oder Jimpster kennt: Moll-9-Stabs, Rhodes, M1-Orgelbass, Gospel-Orgel und
weiche Flächen.

Seit Version 1.1 gibt es außerdem eine eigene Kategorie **Detroit Techno** mit Klängen im Geist
von Juan Atkins, Derrick May, Underground Resistance, Jeff Mills, Carl Craig und Drexciya.

Alle Klänge werden live synthetisiert. Es gibt keine Samples, deshalb ist das Plugin klein und
jeder Regler greift direkt in den Klang ein.

---

## 1. Plugin bauen (einmalig)

AbleDeep wird aus dem Quellcode gebaut. Am einfachsten geht das kostenlos über GitHub, ohne
dass du auf deinem Rechner etwas installieren musst:

1. Lege ein (gern privates) Repository auf github.com an und lade den Inhalt dieses Ordners hoch.
   Wichtig ist, dass der versteckte Ordner `.github` mit hochgeladen wird.
2. Öffne im Repository den Tab **Actions**. Der Build startet automatisch und dauert etwa 10 Minuten.
3. Wenn der Lauf fertig ist, findest du unten unter **Artifacts** die Downloads
   `AbleDeep-Windows` und `AbleDeep-macOS`.

### Alternativ: lokal bauen
Du brauchst CMake (3.22 oder neuer) und Visual Studio 2022 (Windows) bzw. Xcode (Mac).
```
cmake -B build
cmake --build build --config Release
```
JUCE wird dabei automatisch heruntergeladen. Die fertigen Dateien liegen danach in
`build/AbleDeep_artefacts/Release/`.

## 2. Installieren

| System  | Datei                 | Zielordner                                   |
|---------|-----------------------|----------------------------------------------|
| Windows | `AbleDeep.vst3`       | `C:\Program Files\Common Files\VST3\`        |
| macOS   | `AbleDeep.vst3`       | `/Library/Audio/Plug-Ins/VST3/`              |
| macOS   | `AbleDeep.component`  | `/Library/Audio/Plug-Ins/Components/` (AU)   |

**Hinweis für den Mac:** Das Plugin ist nicht von Apple notarisiert. Falls macOS es blockiert,
führe einmal im Terminal Folgendes aus:
```
sudo xattr -dr com.apple.quarantine /Library/Audio/Plug-Ins/VST3/AbleDeep.vst3
sudo xattr -dr com.apple.quarantine /Library/Audio/Plug-Ins/Components/AbleDeep.component
```

**In Ableton:** Öffne *Einstellungen → Plug-Ins*, aktiviere „VST3 Plug-In Systemordner
verwenden“ und klicke auf *Neu scannen*. AbleDeep erscheint danach im Browser unter
*Plug-Ins → AbleDeep Audio*.

---

## 3. Presets

| # | Preset | Typ | Idee |
|---|--------|-----|------|
| 1 | Chandler Stab m9 | Chord-Stab | Klassischer Moll-9-Stab, eine Taste spielt den ganzen Akkord |
| 2 | Velvet Rhodes | E-Piano | Allround-Rhodes mit Tremolo |
| 3 | Soul Keys m11 | E-Piano + Chord | Jazzige m11-Voicings im Atjazz-Stil |
| 4 | Dusty Keys maj9 | E-Piano + Chord | Lo-Fi-Rhodes mit Vinyl-Knistern |
| 5 | M1 Organ Bass | Bass | Perkussiver Orgelbass, das Markenzeichen von NY-Deep-House |
| 6 | Sunday Gospel Organ | Orgel | Volle Gospel-Orgel für Soulful House |
| 7 | Garage Organ Stab m7 | Orgel + Chord | Kurzer Orgel-Akkord mit Delay |
| 8 | Deep Sub | Bass | Sauberer Sub-Bass mit leichtem Glide |
| 9 | Warm Pluck Bass | Bass | Gezupfter Analog-Bass |
| 10 | Midnight Pad | Pad | Atmosphärische Fläche im Jimpster-Stil |
| 11 | Disco Strings | Pad | Breites String-Ensemble |
| 12 | Jazz Pluck | Lead/Pluck | Kurzer Pluck mit punktiertem Delay |
| 13 | Detroit Stab m7 | Chord-Stab | Resonanter, härterer Stab |
| 14 | Deep Stack Swell | Pad + Chord | Offene Akkordfläche mit Filter-Swell |
| 15 | Sunshine Rhodes maj9 | E-Piano + Chord | Sonniger Dur-9-Akkord |
| 16 | Init | – | Neutraler Startpunkt zum Selberbauen |

### Detroit Techno (ab v1.1)

| # | Preset | Typ | Idee |
|---|--------|-----|------|
| 17 | DT Piano Stab | E-Piano + Chord | Heller, harter Piano-Akkord im Stil von Derrick May |
| 18 | DT Stab Strings m7 | Chord-Stab | Kurze Streicher-Stabs, Detroit-Klassiker der späten 80er |
| 19 | DT Motor City Strings | Strings | Großes String-Ensemble für Läufe und Melodien |
| 20 | DT Hi-Tech Brass dom9 | Chord-Stab | Saw-Bläser mit Filter-Anstieg im Stil von UR „Hi-Tech Jazz“ |
| 21 | DT Dub Chord m7 | Chord-Stab | Minimaler Dub-Akkord mit langem Echo (Robert Hood) |
| 22 | DT Hypnotic Pluck | Sequenz | Resonanter Loop-Pluck im Stil von Jeff Mills |
| 23 | DT Cosmic Pad | Pad + Chord | Wandernde, kosmische Fläche (Carl Craig) |
| 24 | DT Bleep Lead | Lead | Bleeps und Sequenzer-Melodien (Juan Atkins / Model 500) |
| 25 | DT Electro Bass | Bass | Knurrender Electro-Bass (Drexciya) |
| 26 | DT Acid Line | Bass | 303-artige Acid-Linie mit Slides; mit dem Mod-Wheel aufdrehen |
| 27 | DT Rumble Bass | Bass | Tiefer, satter Bass unter der 909-Kick |
| 28 | DT Robot Organ 7sus4 | Orgel + Chord | Kalte, perkussive Orgel mit Delay |

**Detroit-Tipp:** Lass die Stabs auf den Offbeats laufen und automatisiere bei *DT Hypnotic
Pluck* und *DT Acid Line* Cutoff und Resonance über viele Takte. Bei *DT Dub Chord* lohnt es
sich, den Delay-Feedback-Regler live hochzuziehen.

Die Presets stehen oben im Plugin zur Auswahl, nach Kategorien sortiert, du kannst aber auch über Abletons Preset-Menü
wechseln. Wenn du eigene Einstellungen behalten willst, speicherst du sie in Ableton als
Instrument-Rack- oder Device-Preset.

## 4. Bedienung

- **Engine**: *Analog* (subtraktiver Synth), *E-Piano* (FM-Rhodes) oder *Organ* (Zugriegel-Orgel).
  Die Sektionen der gerade nicht aktiven Engines werden abgedunkelt.
- **Chord** (Chord Memory): Eine Taste spielt einen ganzen Akkord (min7, min9, min11, maj7,
  maj9, dom9, 7sus4, Deep Stack). Das ist der klassische Trick für Deep-House-Stabs.
- **Voice Mode**: *Mono* mit **Glide** für Basslines.
- **Filter**: 4-Pol-Ladder-Tiefpass mit eigener Hüllkurve. Das **Mod-Wheel** öffnet das Filter
  zusätzlich, damit lassen sich Filterfahrten live spielen.
- **Effects**: Warmth (Sättigung), Juno-Chorus, Tremolo/Auto-Pan, Ping-Pong-Delay (synchron zum
  Ableton-Tempo), Hall (mit Bass-Hochpass), Vinyl Dust.
- Ein Doppelklick auf einen Regler setzt ihn zurück. Sustain-Pedal und Pitch-Bend (±2 Halbtöne)
  werden unterstützt.

**Tipp:** Lege die Chord-Stabs auf die Offbeats, das Delay auf 3/16 und automatisiere Cutoff
über 8 bis 16 Takte. Damit hast du sofort die typische Deep-House-Bewegung.

---

## Projektstruktur
```
Source/AbleDeepEngine.h    Klang-Engine + Presets (reines C++, ohne Framework)
Source/PluginProcessor.*   Plugin-Wrapper (JUCE)
Source/PluginEditor.*      Oberfläche
tools/render_demos.cpp     Rendert Hörproben aller Presets als WAV
.github/workflows/build.yml  Automatischer Build für Windows + macOS
```
Eigene Presets fügst du in `makePresets()` in `AbleDeepEngine.h` hinzu.

Lizenz-Hinweis: AbleDeep nutzt das JUCE-Framework (Version 8). JUCE steht unter der AGPLv3 oder
einer kommerziellen Lizenz. Für die eigene Nutzung ist das unproblematisch. Wenn du das Plugin
weitergeben oder verkaufen willst, prüfe bitte vorher die Lizenzbedingungen auf juce.com.
