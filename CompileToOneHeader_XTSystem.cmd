
echo #pragma once > XTSystem.h
echo #pragma warning(disable: 4482) >> XTSystem.h


type XTSystem_00fmt.h >> XTSystem.h
type XTSystem_01temparg_beg.h >> XTSystem.h
type XTSystem_01aStackWalker.h >> XTSystem.h
type XTSystem_02string.h >> XTSystem.h
type XTSystem_03Exceptions.h >> XTSystem.h
type XTSystem_16enums.h >> XTSystem.h
type XTSystem_04Math.h >> XTSystem.h
type XTSystem_05primitivetypes.h >> XTSystem.h
type XTSystem_06console.h >> XTSystem.h
type XTSystem_07convert.h >> XTSystem.h
type XTSystem_08Text.h >> XTSystem.h
type XTSystem_09io.h >> XTSystem.h
type XTSystem_10Random.h >> XTSystem.h
type XTSystem_11Diagnostics.h >> XTSystem.h
type XTSystem_12Environment.h >> XTSystem.h
rem type XTSystem_13dirent.h >> XTSystem.h
type XTSystem_14ioDirectory.h >> XTSystem.h

type XTSystem_17generics.h >> XTSystem.h
type XTSystem_18registry.h >> XTSystem.h
type XTSystem_19DateTime.h >> XTSystem.h
type XTSystem_20process.h >> XTSystem.h
type XTSystem_21threading.h >> XTSystem.h
type XTSystem_22hresults.h >> XTSystem.h
type XTSystem_23drawing.h >>XTSystem.h

type XTSystem_99temparg_end.h >> XTSystem.h

echo #pragma warning(default: 4482) >> XTSystem.h

