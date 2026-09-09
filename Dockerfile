# Runtime image only: Fedora 44 plus shared libraries the stock image lacks
# (pcre, libicu). Do not install compilers or build the C++ app here.
FROM fedora:44
RUN dnf install -y pcre libicu && dnf clean all
