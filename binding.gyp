{
  "targets": [
    {
      "target_name": "node_opendds",
      "sources": [ "src/node-opendds.cpp",
                   "src/NodeDRListener.cpp",
                   "src/NodeQosConversion.cpp" ],
      "cflags_cc!": [ "-fno-exceptions", "-fno-rtti" ],
      "defines": [ "_REENTRANT", "ACE_HAS_AIO_CALLS", "_GNU_SOURCE",
                   "ACE_HAS_EXCEPTIONS", "__ACE_INLINE__" ],
      "include_dirs" : [ "$(ACE_ROOT)", "$(TAO_ROOT)", "$(DDS_ROOT)" ],
      "link_settings" : {
        "libraries": [ "-lOpenDDS_Dcps", "-lTAO_PortableServer",
                       "-lTAO_AnyTypeCode", "-lTAO", "-lACE" ],
        "ldflags": [ "-L$(ACE_ROOT)/lib", "-L$(DDS_ROOT)/lib" ],
      },
    }
#TODO: configuration variability => OS, debug, optimize, inline
  ]
}
