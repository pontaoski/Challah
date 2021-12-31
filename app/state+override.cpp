#include <QQmlEngine>

#include "overrides_model.h"
#include "state_p.h"

OverridesModel* State::overridesModel()
{
    if (!d->overridesModel) {
        d->overridesModel = new OverridesModel(this, this);
    }

    return d->overridesModel;
}