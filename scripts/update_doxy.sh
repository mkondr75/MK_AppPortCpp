#!/bin/bash
FILE="Doxyfile"
# Массовая замена настроек в файле
sed -i 's/^PROJECT_NAME           =.*/PROJECT_NAME           = "AppPortCpp"/' $FILE
sed -i 's/^OUTPUT_DIRECTORY       =.*/OUTPUT_DIRECTORY       = .\/doc\/doxy/' $FILE
sed -i 's/^OPTIMIZE_OUTPUT_FOR_C  =.*/OPTIMIZE_OUTPUT_FOR_C  = YES/' $FILE
sed -i 's/^EXTRACT_ALL            =.*/EXTRACT_ALL            = YES/' $FILE
sed -i 's/^HAVE_DOT               =.*/HAVE_DOT               = YES/' $FILE
sed -i 's/^RECURSIVE              =.*/RECURSIVE              = YES/' $FILE
sed -i 's/^SOURCE_BROWSER         =.*/SOURCE_BROWSER         = YES/' $FILE
# Включение специфичных для Graphviz опций
sed -i 's/^CALL_GRAPH             =.*/CALL_GRAPH             = YES/' $FILE
sed -i 's/^CALLER_GRAPH           =.*/CALLER_GRAPH           = YES/' $FILE
sed -i 's/^DIRECTORY_GRAPH        =.*/DIRECTORY_GRAPH        = YES/' $FILE
sed -i 's/^INTERACTIVE_SVG        =.*/INTERACTIVE_SVG        = YES/' $FILE

echo "Doxyfile updated and sanitized."
