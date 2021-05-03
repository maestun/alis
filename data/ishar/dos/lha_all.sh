brew install lha
for f in *.io; do lha a $f.lha $f; done
