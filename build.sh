
echo "Building files with GCCXML ..."
gccxml src/all.h -fxml=./data/dictionary_without_dtd.xml
echo "Adding dtd to xml dictionary ..."
xsltproc -o ./data/dictionary.xml ./data/adddtd.xsl ./data/dictionary_without_dtd.xml

