# C++ Ninja VM 

Dieses Repository enthält eine Implementierung der Ninja VM (NJVM) geschrieben in C++ mit dem C++20 Standard.

**Wichtig:** Es ist nicht möglich, diese Implementierung als Teil des Moduls *CS1018 – Konzepte systemnaher Programmierung* abzugeben.
Das Abgabesystem unterstützt weder das Einreichen noch das Bewerten von Abgaben geschrieben in C++.
Zudem ist das Einreichen einer fremden Lösung unter dem eigenen Namen **nicht erlaubt**, wird als **Betrugsversuch** gewertet und kann im schlimmsten Fall zur **Exmatrikulation** führen.

Dieses Projekt ist in erster Linie aus privatem Zweck entstanden.
Es soll anhand bekannter Aufgaben den Umgang mit C++ schulen.
Darüber hinaus soll die hier vorhandene Implementierung strukturiert und ordentlich genug sein, um anderen Hilfestellung zu leisten.
Zusätzlich vom Sprachumfang von C, erlaubt C++ die Verwendung von Referenzparametern, Klassen, Exceptions und generischen Funktionen über Templates, was der Strukturierung dieses Projektes hilft.
Im Umkehrschluss kann (und sollte!) aber der Code hier nicht direkt für eine Implementierung in C übernommen werden.
Vielmehr kann die Implementierung zur Aufklärung dienen oder als Beispiel, welche Strukturen in der NJVM vorkommen und wie diese umgesetzt werden können.
Die tatsächliche Umsetzung der NJVM im Rahmen des Bachelorstudiums sollte von jedem Studierenden selbst geschehen.


## Struktur des Projektes

Die vorhandene NJVM implementiert den vollständigen Funktionsumfang.
Zahlen, Arrays und Records werden als Objekte dynamischer Größe auf einem verwalteten Heap angelegt und mittels Garbage Collection aufgeräumt.
Dazu wird ein Stop-and-Copy verfahren verwendet.
Lediglich ein Debugger, dessen Funktionsumfang den Studierenden überlassen wird, ist nicht implementiert.

Das Projekt selbst ist auf mehrere Dateien aufgeteilt, welche die einzelnen Funktionen der NJVM umfassen:

- [njvm.h](njvm.h) und [types.h](types.h) stellen die Basisdefinitionen für die NJVM bereit.
  Während die [njvm.h](njvm.h) Datei die Register, Komponenten und Hilfsdefinitionen für die Maschine anbietet, werden in [types.h](types.h) alle Typen definiert, die von der NJVM verwendet werden.
  Dabei handelt es sich sowohl um Typen für das Laden und Auswerten von Instruktionen, als auch für die Abbildung der Ninja-Objekte in C++.

- [loader.h](loader.h) stellt den Lader für Binärdateien bereit.
  [Die Implementierung](loader.cpp) ist einfach gehalten und dem C-Stil nachempfunden.
  Bis auf Exceptions werden hier keine Features von C++ verwendet.

- [instructions.h](instructions.h) stellt Definitionen für den Umgang mit Instruktionen bereit.
  In der [zugehörigen Implementierung](instructions.cpp) wird besonders die `constexpr` Funktionalität von C++ verwendet, um Berechnungen zur Compilezeit auszuführen.
  So wird die Semantik der Instruktionen per Mnemonic assoziiert anstelle eines maschinenlesbaren Opcodes.

- [gc.h](gc.h) beinhaltet die Schnittstelle zum Garbage-Collector und der Heap-Verwaltung.
  Wie in der Vorlesung besprochen wird hier das Stop-and-Copy Verfahren implementiert um ungenutzte Objekte vom Heap aufzuräumen, falls für das Anlegen neuer Objekte nicht mehr genügend Speicher vorhanden ist.

---

Copyright (C) 2022, Niklas Deworetzki
