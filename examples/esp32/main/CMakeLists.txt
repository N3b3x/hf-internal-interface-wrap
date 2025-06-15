idf_component_register(SRCS "main.cpp"
                            "DummyTransport.cpp" INCLUDE_DIRS "." REQUIRES hf -
                       internal - interface - wrap)
