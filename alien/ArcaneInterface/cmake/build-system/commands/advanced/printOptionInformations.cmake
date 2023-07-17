function(printOptionInformations)

  get_property(options GLOBAL PROPERTY ${PROJECT_NAME}_OPTIONS)
   
  foreach(option ${options})
    if(${option}_IS_BOOLEAN)
      if(${option})
        logStatus(" ** ${Green}ON${ColourReset}  : ${${option}_MESSAGE} [${Blue}-D${${option}_COMMANDLINE}=ON${ColourReset}]")
      else()
        logStatus(" ** ${Red}OFF${ColourReset} : ${${option}_MESSAGE} [${Blue}-D${${option}_COMMANDLINE}=OFF${ColourReset}]")
      endif()
    endif()
  endforeach()

  foreach(option ${options})
    if(NOT ${option}_IS_BOOLEAN)
      if(${option})
        logStatus(" ** ${Green}DEF${ColourReset} : ${${option}_MESSAGE} [${Blue}-D${${option}_COMMANDLINE}=${${option}_VALUE}${ColourReset}]")
      else()
        logStatus(" ** ${Red}ABS${ColourReset} : ${${option}_MESSAGE}")
      endif()
    endif()
  endforeach()

endfunction()