#include <QCoreApplication>
#include <QDebug>

#include "testobjects.h"
#include "xmlmapper.h"

/* Use Introspection */
static void test1 (void) {
	QString xml = 	"<contact>"
						"<age>21</age>"
						"<name>Matteo</name>"
						"<surname>Bertozzi</surname>"
						"<website>http://th30z.blogspot.com/</website>"
					"</contact>";

	TestObject obj;
	THXmlMapper xmlMapper1(&obj);
	xmlMapper1.map(xml);
	qDebug() << obj.name() << obj.surname() << obj.age() << obj.website();
}

/* Use Mapping */
static void test2 (void) {
	QString xml = 	"<personInfo>"
						"<personAge>21</personAge>"
						"<personName>Matteo</personName>"
						"<personSurname>Bertozzi</personSurname>"
						"<personWeb>http://th30z.blogspot.com/</personWeb>"
					"</personInfo>";

	THXmlHashMap dataMapping;
	dataMapping.insert("personAge", "age");
	dataMapping.insert("personName", "name");
	dataMapping.insert("personSurname", "surname");
	dataMapping.insert("personWeb", "website");

	TestObject obj;
	THXmlMapper xmlMapper(&obj);
	xmlMapper.setMapping(dataMapping);
	xmlMapper.map(xml);
	qDebug() << obj.name() << obj.surname() << obj.age() << obj.website();
}

/* Use Mapping/Introspection */
static void test3 (void) {
	QString xml = 	"<personInfo>"
						"<age>21</age>"
						"<personName>Matteo</personName>"
						"<surname>Bertozzi</surname>"
						"<personWeb>http://th30z.blogspot.com/</personWeb>"
					"</personInfo>";

	THXmlHashMap dataMapping;
	dataMapping.insert("personName", "name");
	dataMapping.insert("personWeb", "website");

	TestObject obj;
	THXmlMapper xmlMapper(&obj);
	xmlMapper.setMapping(dataMapping);
	xmlMapper.map(xml);
	qDebug() << obj.name() << obj.surname() << obj.age() << obj.website();
}

int main (int argc, char **argv) {
	QCoreApplication app(argc, argv);

	test1();		// Only Introspection
	test2();		// Only Mapping
	test3();		// Mapping & Introspection

	return(0);
}

