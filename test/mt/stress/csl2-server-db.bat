server -ORBIIOPAddr ssl:inet:localhost:12466 -ORBSSLcert s_cert.pem -ORBSSLkey s_key.pem -ORBSSLverify 0 -ODMConfig odm.cnf -AccessConfig access.cnf -AuditConfig audit.cnf -RightsConfig rights.cnf -AuditType db -AuditArchName dbname=csl2-audit -Paranoid yes

