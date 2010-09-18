#include <QMetaProperty>
#include <QMetaObject>

#include <QDateTime>
#include <QTime>
#include <QUrl>

#include "xmlmapper.h"

/* ============================================================================
 *  PRIVATE Class
 */
class THXmlMapperPrivate {
	public:
		bool useIntrospection;
		THXmlHashMap mapping;
		QObject *entity;

	public:
		int propertyIndex (const QString& name);

		QVariant::Type propertyType (int index);
		QVariant::Type propertyType (const QString& name);

		void setProperty (int index, const QVariant& value);
		void setProperty (const QString& name, const QVariant& value);
};

int THXmlMapperPrivate::propertyIndex (const QString& name) {
	if (!useIntrospection)
		return(-1);

	return(entity->metaObject()->indexOfProperty(name.toAscii().constData()));
}

QVariant::Type THXmlMapperPrivate::propertyType (int index) {
	if (index < 0) return(QVariant::Invalid);
	return(entity->metaObject()->property(index).type());
}

QVariant::Type THXmlMapperPrivate::propertyType (const QString& name) {
	return(propertyType(propertyIndex(name)));
}

void THXmlMapperPrivate::setProperty (int index, const QVariant& value) {
	if (index >= 0) entity->metaObject()->property(index).write(entity, value);
}

void THXmlMapperPrivate::setProperty (const QString& name, const QVariant& value) {
	setProperty(propertyIndex(name), value);
}

/* ============================================================================
 *  PUBLIC Constructor/Destructor
 */
THXmlMapper::THXmlMapper()
	: d(new THXmlMapperPrivate)
{
	d->entity = NULL;
	d->useIntrospection = true;
}

THXmlMapper::THXmlMapper (QObject *entity)
	: d(new THXmlMapperPrivate)
{
	d->entity = entity;
	d->useIntrospection = true;
}

THXmlMapper::THXmlMapper (QObject *entity, const THXmlHashMap& mapping)
	: d(new THXmlMapperPrivate)
{
	d->entity = entity;
	d->mapping = mapping;
	d->useIntrospection = true;
}

THXmlMapper::THXmlMapper (const THXmlHashMap& mapping)
	: d(new THXmlMapperPrivate)
{
	d->entity = NULL;
	d->mapping = mapping;
	d->useIntrospection = true;
}

THXmlMapper::~THXmlMapper() {
	delete d;
}


/* ============================================================================
 *  PUBLIC Methods
 */
void THXmlMapper::map (QIODevice *device) {
	QXmlStreamReader xmlReader(device);
	map(&xmlReader);
}

void THXmlMapper::map (const QString& xml) {
	QXmlStreamReader xmlReader(xml);
	while (!xmlReader.atEnd()) {
		xmlReader.readNext();
		if (xmlReader.isStartElement())
			break;
	}

	map(&xmlReader);
}

void THXmlMapper::map (const QByteArray& xml) {
	QXmlStreamReader xmlReader(xml);
	while (!xmlReader.atEnd()) {
		xmlReader.readNext();
		if (xmlReader.isStartElement())
			break;
	}
	map(&xmlReader);
}

void THXmlMapper::map (QXmlStreamReader *xmlReader) {
	while (!xmlReader->atEnd()) {
		xmlReader->readNext();

		if (xmlReader->isEndElement())
			break;

		if (xmlReader->isStartElement())
			mapXmlNode(xmlReader);
	}
}


/* ============================================================================
 *  PUBLIC Properties
 */
THXmlHashMap& THXmlMapper::mapping (void) const {
	return(d->mapping);
}

void THXmlMapper::setMapping (const THXmlHashMap& mapping) {
	d->mapping = mapping;
}


QObject *THXmlMapper::entity (void) const {
	return(d->entity);
}

void THXmlMapper::setEntity (QObject *entity) {
	d->entity = entity;
}


bool THXmlMapper::useIntrospection (void) {
	return(d->useIntrospection);
}

void THXmlMapper::setUseIntrospection (bool useIntrospection) {
	d->useIntrospection = useIntrospection;
}

/* ============================================================================
 *  PROTECTED Methods
 */
void THXmlMapper::mapXmlNode (QXmlStreamReader *xmlReader) {
	QString tagName = xmlReader->name().toString();
	int pIndex;

	if (d->mapping.contains(tagName)) {
		tagName = d->mapping[tagName];
		QString data = readXmlNode(xmlReader);
		d->setProperty(tagName, convertData(d->propertyType(tagName), data));
	} else if ((pIndex = d->propertyIndex(tagName)) >= 0) {
		QString data = readXmlNode(xmlReader);
		d->setProperty(pIndex, convertData(d->propertyType(pIndex), data));
	} else {
		readUnknownXmlNode(xmlReader);
	}
}

void THXmlMapper::readUnknownXmlNode (QXmlStreamReader *xmlReader) {
	while (!xmlReader->atEnd()) {
		xmlReader->readNext();

		if (xmlReader->isEndElement())
			break;

		if (xmlReader->isStartElement())
			readUnknownXmlNode(xmlReader);
	}
}

QString THXmlMapper::readXmlNode (QXmlStreamReader *xmlReader) {
	QString buffer;

	while (!xmlReader->atEnd()) {
		xmlReader->readNext();

		if (xmlReader->isEndElement())
			break;

		buffer = xmlReader->text().toString();
	}

	return(buffer);
}

QVariant THXmlMapper::convertData (QVariant::Type type, const QString& data) {
	if (type == QVariant::Bool) {
		return(bool(data.compare("true", Qt::CaseInsensitive) ||  data == "1"));
	} else if (type == QVariant::Char) {
		return(data.size() > 0 ? data.at(0) : QChar());
	} else if (type == QVariant::ByteArray) {
		return(data.toAscii());
	} else if (type == QVariant::Date) {
		return(QDateTime::fromString(data));
	} else if (type == QVariant::Double) {
		return(data.toDouble());
	} else if (type == QVariant::Int) {
		return(data.toInt());
	} else if (type == QVariant::LongLong) {
		return(data.toLongLong());
	} else if (type == QVariant::Time) {
		return(QTime::fromString(data));
	} else if (type == QVariant::UInt) {
		return(data.toUInt());
	} else if (type == QVariant::ULongLong) {
		return(data.toULongLong());
	} else if (type == QVariant::Url) {
		return(QUrl(data));
	}

	return(data);
}

