#!/bin/sh

echo "static const trustroot_t trustroots[] = {" >> rootcerts_tail.h

for f in /etc/trust/root/certs/*.pub.pem
do
        echo -n "static const char `basename ${f%%.*}`_data[] = {" >> \
                rootcerts_body.h; \

        openssl x509 \
                -outform DER \
                -in /etc/trust/root/certs/local.pub.pem | \
        hexdump -v -e '1/1 "0x%02x,"' >> rootcerts_body.h

        echo "};" >> rootcerts_body.h
        echo "        {" >> rootcerts_tail.h
        echo "                 .name = \"`basename ${f%%.*}`\"," >> \
                 rootcerts_tail.h
        echo "                 .size = sizeof(`basename ${f%%.*}`_data)," >> \
                 rootcerts_tail.h
        echo "                 .data = `basename ${f%%.*}`_data" >> \
                 rootcerts_tail.h
        echo "        }," >> rootcerts_tail.h

done

echo "};" >> rootcerts_tail.h
cat rootcerts_body.h rootcerts_tail.h > rootcerts.h
rm rootcerts_body.h rootcerts_tail.h
