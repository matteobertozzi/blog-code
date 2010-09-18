#ifndef _THXMLMAPPER_H_
#define _THXMLMAPPER_H_

#include <QXmlStreamReader>
#include <QVariant>
#include <QObject>
#include <QHash>

class THXmlMapperPrivate;

typedef QHash<QString, QString> THXmlHashMap;

class THXmlMapper {
	public:
		THXmlMapper ();
		THXmlMapper (QObject *entity);
		THXmlMapper (QObject *entity, const THXmlHashMap& mapping);
		THXmlMapper (const THXmlHashMap& mapping);
		~THXmlMapper();

		// Methods
		void map (QIODevice *device);
		void map (const QString& xml);
		void map (const QByteArray& xml);
		void map (QXmlStreamReader *xmlReader);

		// Properties
		THXmlHashMap& mapping (void) const;
		void setMapping (const THXmlHashMap& mapping);

		QObject *entity (void) const;
		void setEntity (QObject *entity);

		bool useIntrospection (void);
		void setUseIntrospection (bool useIntrospection);

	protected:
		virtual void mapXmlNode (QXmlStreamReader *xmlReader);
		virtual void readUnknownXmlNode (QXmlStreamReader *xmlReader);

		virtual QString readXmlNode (QXmlStreamReader *xmlReader);
		virtual QVariant convertData (QVariant::Type type, const QString& data);

	private:
		THXmlMapperPrivate *d;
};

#endif /* !_THXMLMAPPER_H_ */

