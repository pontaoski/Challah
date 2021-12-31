#pragma once

#include "client.h"
#include "overrides_model.h"
#include "profile/v1/appdata.pb.h"

struct Tag
{
    Q_GADGET

    Q_PROPERTY(QString before MEMBER before)
    Q_PROPERTY(QString after MEMBER after)

public:
    QString before;
    QString after;

    Tag()
    {

    }
    Tag(const std::string& before, const std::string& after)
        : before(QString::fromStdString(before))
        , after(QString::fromStdString(after))
    {
    }
};

Q_DECLARE_METATYPE(Tag)

struct OverridesModel::Private
{
    QList<protocol::profile::v1::ProfileOverride> overrides;
    bool dirty = false;
};