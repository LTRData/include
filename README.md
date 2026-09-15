# LTR Data shared headers

Shared C and C++ headers used by LTR Data's native Windows tools and libraries. The collection includes WinStructured helpers, NT API declarations, compatibility shims and third-party headers.

Many WinStructured headers date from the early 2000s, with later changes for newer compilers, architectures and Windows APIs. Recent history includes Visual Studio 2022 compatibility and legacy-compiler fixes through February 2026. This repository is consumed by other projects; it has no standalone application, library build or package-install target.

## Contents

| Area | Representative headers | Purpose |
| --- | --- | --- |
| WinStructured core | [winstrct.h](winstrct.h), [winstrct.hpp](winstrct.hpp), [wbase.hpp](wbase.hpp) | Windows error helpers, memory utilities and handle wrappers |
| File and console I/O | [wio.h](wio.h), [wfind.h](wfind.h), [wfilemap.h](wfilemap.h), [wconsole.h](wconsole.h) | File/pipe I/O, directory enumeration, mapping and console helpers |
| Native NT operations | [ntdll.h](ntdll.h), [ntfileio.hpp](ntfileio.hpp), [wntfind.h](wntfind.h) | Native API declarations and file/directory helpers |
| Windows objects | [wreg.hpp](wreg.hpp), [wntsecur.h](wntsecur.h), [wscm.h](wscm.h), [wsync.h](wsync.h), [wthread.hpp](wthread.hpp) | Registry, security, service control, synchronization and threads |
| Communications | [wcomm.h](wcomm.h), [wsocket.hpp](wsocket.hpp) | Serial-port structures and socket wrappers |
| Compatibility material | [unistd.h](unistd.h), [stdint.h](stdint.h), [sys/](sys/), [machine/](machine/) | Selected runtime/POSIX-style declarations and adaptations |
| Third-party and historical variants | [bzlib.h](bzlib.h), [raptor2/raptor2.h](raptor2/raptor2.h), [bjy/ntdll.h](bjy/ntdll.h), [minwcrtlib.h](minwcrtlib.h) | Imported interfaces and compatibility headers with their own provenance |

The root `ntdll.h` and `bjy/ntdll.h` are distinct headers. Use the one expected by the consuming source.

## Using the headers

Keep the checkout named `include` alongside projects that reference `..\include`. Examples include [rawcopy](https://github.com/LTRData/rawcopy), [fdf](https://github.com/LTRData/fdf) and [strarc](https://github.com/LTRData/strarc).

Configure the consuming project's include path to find this directory. Many projects use shared property sheets from [LTRData/props](https://github.com/LTRData/props); its `winstrct.props` adds `..\include` and an architecture-specific library search path. Other projects retain different or absolute property-sheet paths, so follow the actual project or makefile.

This collection includes both inline code and declarations requiring separately compiled implementations. Many corresponding sources are in [LTRData/libsrc](https://github.com/LTRData/libsrc). With MSVC, `winstrct.h` requests `winstrct.lib` and `winstrcp.lib` through linker pragmas unless `NO_WINSTRCT_LIB_IMPORT` is defined. That macro only suppresses those automatic library requests; any referenced non-inline functions still need implementations. Libraries and third-party runtimes are not bundled here.

Use the compiler, Windows SDK/WDK, architecture and character-set definitions expected by the consuming project. Some headers depend on declarations included earlier, and the collection contains compiler-specific code and historical SDK definitions. There is no single supported-toolchain or Windows-version matrix for all files.

## Compatibility boundaries

Add this directory only to projects that need its headers. Names such as `stdint.h`, `intrin.h`, `unistd.h` and `sys/*` can shadow SDK or runtime headers depending on include-path order.

Several files, including `pthread.h`, `dirent.h`, `intrin.h` and `strings.h`, are empty placeholders. Their presence does not supply the corresponding APIs. The POSIX-style material is a set of adaptations for particular builds, not a complete POSIX implementation.

The Raptor header still contains configure-time placeholders such as `@RAPTOR_VERSION_DECIMAL@`. It is not a fully configured SDK header ready for every consumer.

## Documentation

[Doxyfile](Doxyfile) contains the historical **WinStructured 1.10** documentation configuration. It refers to sibling `..\c` and `..\cpp` source directories and a `werrlog.hpp` input absent from this checkout. Update its input paths for the source layout being documented before generating documentation. Individual headers also contain usage comments.

## Provenance and licensing

This is a collection of material from several sources, with no repository-wide license file. Preserve the notices in the files you use and check the terms for those specific files and their linked implementations.

Examples of notices already present:

| Files | Recorded provenance or terms |
| --- | --- |
| [winstrct.h](winstrct.h), [winstrct.hpp](winstrct.hpp) | Olof Lagerkvist's WinStructured helpers |
| [bjy/ntdll.h](bjy/ntdll.h) | Bang Jun-Young; redistribution conditions include an advertising acknowledgement |
| [bzlib.h](bzlib.h) | Julian Seward's bzip2/libbzip2 header and its redistribution notice |
| [raptor2/raptor2.h](raptor2/raptor2.h) | David Beckett / University of Bristol; lists LGPL, GPL and Apache licensing alternatives |
| [sys/cdefs.h](sys/cdefs.h), [machine/_types.h](machine/_types.h) | Berkeley/FreeBSD-derived headers with embedded notices |
| [minwcrtlib.h](minwcrtlib.h) | Carries a Microsoft Corporation copyright notice |
