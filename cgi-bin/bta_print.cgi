#!/bin/bash
echo -e "Access-Control-Allow-Origin: http://ishtar.sao.ru\n"
echo -e "Access-Control-Allow-Methods: POST\n"
echo -e "Content-type: multipart/form-data\n\n"
/usr/local/bin/bta_print 2>/dev/null
