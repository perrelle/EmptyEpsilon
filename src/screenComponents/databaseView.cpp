#include <i18n.h>
#include "databaseView.h"
#include "scienceDatabase.h"

#include "gui/gui2_listbox.h"
#include "gui/gui2_image.h"
#include "gui/gui2_keyvaluedisplay.h"
#include "gui/gui2_scrolltext.h"

#include "screenComponents/rotatingModelView.h"

DatabaseViewComponent::DatabaseViewComponent(GuiContainer* owner)
: GuiElement(owner, "DATABASE_VIEW")
{
    item_list = new GuiListbox(this, "DATABASE_ITEM_LIST", [this](int index, string value) {
        P<ScienceDatabase> entry;

        int32_t id = std::stoul(value, nullptr, 10);
        selected_entry = findEntryById(id);
        display();
    });
    setAttribute("layout", "horizontal");
    item_list->setMargins(20)->setSize(navigation_width, GuiElement::GuiSizeMax);
    display();
}

P<ScienceDatabase> DatabaseViewComponent::findEntryById(int32_t id)
{
    if (id == 0)
    {
        return nullptr;
    }
    for(auto sd: ScienceDatabase::science_databases)
    {
        if (!sd) continue;
        if (sd->getId() == id)
        {
            return sd;
        }
    }
    return nullptr;
}

bool DatabaseViewComponent::findAndDisplayEntry(string name)
{
    for(auto sd : ScienceDatabase::science_databases)
    {
        if (!sd) continue;
        if (sd->getName() == name)
        {
            selected_entry = sd;
            display();
            return true;
        }
    }
    return false;
}

void DatabaseViewComponent::fillListBox()
{
    item_list->setOptions({});
    item_list->setSelectionIndex(-1);

    // indices of child or sibling pages in the science_databases vector
    std::vector<unsigned> children_idx;
    std::vector<unsigned> siblings_idx;
    P<ScienceDatabase> parent_entry;

    for (unsigned idx=0; idx<ScienceDatabase::science_databases.size(); idx++)
    {
        P<ScienceDatabase> sd = ScienceDatabase::science_databases[idx];
        if (!sd) continue;

        if(selected_entry)
        {
            if(sd->getId() == selected_entry->getParentId())
            {
                parent_entry = sd;
            }
            if(sd->getParentId() == selected_entry->getParentId())
            {
                siblings_idx.push_back(idx);
            }
            if(sd->getParentId() == selected_entry->getId())
            {
                children_idx.push_back(idx);
            }
        }
        else
        {
            if(sd->getParentId() == 0)
            {
                siblings_idx.push_back(idx);
            }
        }
    }

    if(selected_entry)
    {
        if (children_idx.size() != 0)
        {
            item_list->addEntry(tr("button", "Back"), std::to_string(selected_entry->getParentId()));
        }
        else if(parent_entry)
        {
            item_list->addEntry(tr("button", "Back"), std::to_string(parent_entry->getParentId()));
        }
    }

    // the indices we actually want to display
    std::vector<unsigned> display_idx = children_idx.size() > 0 ? children_idx : siblings_idx;

    sort(display_idx.begin(), display_idx.end(), [](unsigned idxA, unsigned idxB) -> bool {
        return ScienceDatabase::science_databases[idxA] < ScienceDatabase::science_databases[idxB];
    });

    for (auto idx : display_idx)
    {
        P<ScienceDatabase> sd = ScienceDatabase::science_databases[idx];
        int item_list_idx = item_list->addEntry(sd->getName(), std::to_string(sd->getId()));
        if (selected_entry && selected_entry->getId() == sd->getId())
        {
            item_list->setSelectionIndex(item_list_idx);
        }
    }
}

void DatabaseViewComponent::display()
{
    if (keyvalue_container)
        keyvalue_container->destroy();
    if (details_container)
        details_container->destroy();

    keyvalue_container = new GuiElement(this, "KEY_VALUE_CONTAINER");
    keyvalue_container->setMargins(20)->setSize(400, GuiElement::GuiSizeMax)->setAttribute("layout", "vertical");

    details_container = new GuiElement(this, "DETAILS_CONTAINER");
    details_container->setMargins(20)->setSize(GuiElement::GuiSizeMax, GuiElement::GuiSizeMax)->setAttribute("layout", "vertical");

    fillListBox();

    if (!selected_entry)
        return;

    bool has_key_values = selected_entry->keyValuePairs.size() > 0;
    bool has_image_or_model = selected_entry->hasModelData() || selected_entry->getImage() != "";
    bool has_text = selected_entry->getLongDescription().length() > 0;

    if (has_image_or_model)
    {
        GuiElement* visual = (new GuiElement(details_container, "DATABASE_VISUAL_ELEMENT"))->setMargins(0, 0, 0, 40)->setSize(GuiElement::GuiSizeMax, GuiElement::GuiSizeMatchWidth);

        if (selected_entry->hasModelData())
        {
            (new GuiRotatingModelView(visual, "DATABASE_MODEL_VIEW", selected_entry->getModelData()))->setSize(GuiElement::GuiSizeMax, GuiElement::GuiSizeMax);
            if(selected_entry->getImage() != "")
            {
                (new GuiImage(visual, "DATABASE_IMAGE", selected_entry->image))->setMargins(0)->setSize(32, 32);
            }
        }
        else if(selected_entry->getImage() != "")
        {
            auto image = new GuiImage(visual, "DATABASE_IMAGE", selected_entry->image);
            image->setSize(GuiElement::GuiSizeMax, GuiElement::GuiSizeMax);
        }
    }
    if (has_text)
    {
        (new GuiScrollText(details_container, "DATABASE_LONG_DESCRIPTION", selected_entry->getLongDescription()))->setTextSize(24)->setSize(GuiElement::GuiSizeMax, GuiElement::GuiSizeMax);
    }

    if (has_key_values)
    {
        for(unsigned int n=0; n<selected_entry->keyValuePairs.size(); n++)
        {
            (new GuiKeyValueDisplay(keyvalue_container, "", 0.37, selected_entry->keyValuePairs[n].key, selected_entry->keyValuePairs[n].value))->setSize(GuiElement::GuiSizeMax, 40);
        }
    } else {
        keyvalue_container->destroy();
        keyvalue_container = nullptr;
    }
}
