
#include "bench.h"
#include <CORBA.h>
#include <iostream>
#include <string>
#include <unistd.h>


using namespace std;

CORBA::Long ip_count;
CORBA::Long ex_count;
CORBA::Long point;

class MyClientInterceptor
    : virtual public PortableInterceptor::ClientRequestInterceptor
{
    string nm;
public:
    MyClientInterceptor()
    { nm = ""; }

    MyClientInterceptor(const char * name)
    { nm = name; }
	
    char*
    name()
    { return CORBA::string_dup(nm.c_str()); }

    void destroy()
    { cerr << this->name() << " destroy" << endl; }

    void
    send_request(PortableInterceptor::ClientRequestInfo_ptr ri)
    {
	cout << this->name() << "::send_request" << endl;
	ip_count++;
	if (point == ip_count) {
	    if (ex_count == 0) {
		CORBA::NO_PERMISSION ex;
		throw ex;
	    }
	    if (ex_count == 1) {
		Exc ex;
		throw ex;
	    }
	    if (ex_count == 2) {
		throw 10;
	    }
	}
    }
    
    void
    send_poll(PortableInterceptor::ClientRequestInfo_ptr ri)
    {}
    
    void
    receive_reply(PortableInterceptor::ClientRequestInfo_ptr ri)
    {
	cout << this->name() << "::receive_reply" << endl;
	ip_count++;
	if (point == ip_count) {
	    if (ex_count == 0) {
		CORBA::NO_PERMISSION ex;
		throw ex;
	    }
	    if (ex_count == 1) {
		Exc ex;
		throw ex;
	    }
	    if (ex_count == 2) {
		throw 10;
	    }
	}
    }
    
    void
    receive_exception(PortableInterceptor::ClientRequestInfo_ptr ri)
    {
	cout << this->name() << "::receive exception: " << ri->received_exception_id() << endl;
    }
    
    void
    receive_other(PortableInterceptor::ClientRequestInfo_ptr ri)
    {}
};

class MyInitializer
    : virtual public PortableInterceptor::ORBInitializer
{
public:
    MyInitializer()
    {}
    ~MyInitializer()
    {}

    virtual void
    pre_init(PortableInterceptor::ORBInitInfo_ptr info)
    {
    	// register interceptor	
    	MyClientInterceptor * interceptor1 = new MyClientInterceptor("MyInterceptor1");
    	info->add_client_request_interceptor(interceptor1);
    	MyClientInterceptor * interceptor2 = new MyClientInterceptor("MyInterceptor2");
    	info->add_client_request_interceptor(interceptor2);
    	MyClientInterceptor * interceptor3 = new MyClientInterceptor("MyInterceptor3");
    	info->add_client_request_interceptor(interceptor3);
    }
    
    virtual void
    post_init(PortableInterceptor::ORBInitInfo_ptr info)
    {}
};

int
main (int argc, char* argv[])
{
    ip_count = 0;
    ex_count = 0;
    point = 0;
    MyInitializer * ini = new MyInitializer;
    PortableInterceptor::register_orb_initializer(ini);
    CORBA::ORB_ptr orb = CORBA::ORB_init (argc, argv, "mico-local-orb");

    char pwd[256], uri[300];
    sprintf (uri, "file://%s/ref", getcwd(pwd, 256));

    CORBA::Object_var obj = orb->string_to_object(uri);
    bench_var bench = bench::_narrow(obj);
    if (CORBA::is_nil(bench)) {
	cout << "oops: could not locate bench server" << endl;
	exit (1);
    }
    // testing client side
    cout << "client: testing client side..." << endl;
    for (int i=0; i<3; i++) {
	ex_count = i;
	point = 0;
	for (int j=0; j<6; j++) {
	    try {
		point++;
		bench->perform();
	    } catch (CORBA::Exception &ex) {
		cout << "client: " << ex._repoid() << endl;
	    }
	    ip_count = 0;
	}
    }
    ip_count = 1;
    point = 0;
    // testing server side
    cout << "client: testing server side..." << endl;
    bench->start_test();
    for (int i=0; i<27; i++) {
	try {
	    bench->perform();
	} catch (CORBA::Exception &ex) {
	    cout << "client: " << ex._repoid() << endl;
	}
    }
    return 0;
}

