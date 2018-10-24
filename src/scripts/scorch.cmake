set(EXTENSIONS_TO_SCORCH exe dll pdb ilk pch obj h cpp)

foreach(EXTENSION in ${EXTENSIONS_TO_SCORCH})
    file(GLOB_RECURSE FILES_TO_DELETE "${folder}/*.${EXTENSION}")
    
    foreach (FILE_TO_DELETE in ${FILES_TO_DELETE})
        file(REMOVE ${FILE_TO_DELETE})
    endforeach()
endforeach()

