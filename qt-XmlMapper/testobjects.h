#ifndef _TEST_OBJECTS_H_
#define _TEST_OBJECTS_H_

#include <QObject>
#include <QUrl>

class TestObject : public QObject {
	Q_OBJECT

	Q_PROPERTY(QString surname READ surname WRITE setSurname)
	Q_PROPERTY(QUrl website READ website WRITE setWebSite)
	Q_PROPERTY(QString name READ name WRITE setName)
	Q_PROPERTY(int age READ age WRITE setAge)

	public:
		TestObject (QObject *parent = 0) : QObject(parent) {}
		~TestObject() {}

		int age (void) const { return(m_age); }
		void setAge (int age) { m_age = age; }

		QUrl website (void) const { return(m_website); }
		void setWebSite (const QUrl& webSite) { m_website = webSite; }

		QString name (void) const { return(m_name); }
		void setName (const QString& name) { m_name = name; }

		QString surname (void) const { return(m_surname); }
		void setSurname (const QString& surname) { m_surname = surname; }

	private:
		QString m_surname;
		QString m_name;
		QUrl m_website;
		int m_age;
};

#endif /* !_TEST_OBJECTS_H_ */

