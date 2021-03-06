# group files
macro(GroupFiles fileGroup)
    string(REPLACE "_" " " fileGroupName ${fileGroup})
    string(TOLOWER ${fileGroupName} fileGroupName)
    string(REGEX MATCHALL "([^ ]+)" fileGroupNameSplit ${fileGroupName})

    set(finalFileGroupName)
    foreach(fileGroupNameWord ${fileGroupNameSplit})
        string(SUBSTRING ${fileGroupNameWord} 0 1 firstLetter)
        string(SUBSTRING ${fileGroupNameWord} 1 -1 otherLetters)
        string(TOUPPER ${firstLetter} firstLetter)
        if(finalFileGroupName)
            set(finalFileGroupName "${finalFileGroupName} ")
        endif()
        set(finalFileGroupName "${finalFileGroupName}${firstLetter}${otherLetters}")
    endforeach()

    foreach(currentFile ${${fileGroup}})
        set(folder ${currentFile})
        get_filename_component(filename ${folder} NAME)
        string(REPLACE "${filename}" "" folder ${folder})
        set(groupName "${finalFileGroupName}")
        if(NOT folder STREQUAL "")
            string(REGEX REPLACE "/+$" "" baseFolder ${folder})
            string(REPLACE "/" "\\" baseFolder ${baseFolder})
            set(groupName "${groupName}\\${baseFolder}")
        endif()
        source_group("${groupName}" FILES ${currentFile})
    endforeach()
endmacro()
