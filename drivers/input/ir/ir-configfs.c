/*
 * Configfs routines for IR support
 *
 *   configfs root
 *   --remotes
 *   ----specific remote
 *   ------keymap
 *   --------protocol
 *   --------device
 *   --------command
 *   --------keycode
 *   ------repeat keymaps
 *   --------....
 *   ----another remote
 *   ------more keymaps
 *   --------....
 *
 * Copyright (C) 2008 Jon Smirl <jonsmirl@gmail.com>
 */

#include <linux/kernel.h>
#include <linux/device.h>
#include <linux/input.h>

#include "ir.h"

struct keymap {
	struct config_item item;
	int protocol;
	int device;
	int command;
	int keycode;
};

static inline struct keymap *to_keymap(struct config_item *item)
{
	return item ? container_of(item, struct keymap, item) : NULL;
}

struct remote {
	struct config_group group;
	struct input_dev *input;
};

static inline struct remote *to_remote(struct config_group *group)
{
	return group ? container_of(group, struct remote, group) : NULL;
}


static struct configfs_attribute item_protocol = {
	.ca_owner = THIS_MODULE,
	.ca_name = "protocol",
	.ca_mode = S_IRUGO | S_IWUSR,
};

static struct configfs_attribute item_device = {
	.ca_owner = THIS_MODULE,
	.ca_name = "device",
	.ca_mode = S_IRUGO | S_IWUSR,
};

static struct configfs_attribute item_command = {
	.ca_owner = THIS_MODULE,
	.ca_name = "command",
	.ca_mode = S_IRUGO | S_IWUSR,
};

static struct configfs_attribute item_keycode = {
	.ca_owner = THIS_MODULE,
	.ca_name = "keycode",
	.ca_mode = S_IRUGO | S_IWUSR,
};

static ssize_t item_show(struct config_item *item,
				      struct configfs_attribute *attr,
				      char *page)
{
	struct keymap *keymap = to_keymap(item);

	if (attr == &item_protocol)
		return sprintf(page, "%d\n", keymap->protocol);
	if (attr == &item_device)
		return sprintf(page, "%d\n", keymap->device);
	if (attr == &item_command)
		return sprintf(page, "%d\n", keymap->command);
	return sprintf(page, "%d\n", keymap->keycode);
}

static ssize_t item_store(struct config_item *item,
				       struct configfs_attribute *attr,
				       const char *page, size_t count)
{
	struct keymap *keymap = to_keymap(item);
	struct remote *remote;
	unsigned long tmp;
	char *p = (char *) page;

	tmp = simple_strtoul(p, &p, 10);
	if (!p || (*p && (*p != '\n')))
		return -EINVAL;

	if (tmp > INT_MAX)
		return -ERANGE;

	if (attr == &item_protocol)
		keymap->protocol = tmp;
	else if (attr == &item_device)
		keymap->device = tmp;
	else if (attr == &item_command)
		keymap->command = tmp;
	else {
		if (tmp < KEY_MAX) {
			remote = to_remote(to_config_group(item->ci_parent));
			set_bit(tmp, remote->input->keybit);
			keymap->keycode = tmp;
		}
	}
	return count;
}

static void keymap_release(struct config_item *item)
{
	struct keymap *keymap = to_keymap(item);
	struct remote *remote = to_remote(to_config_group(item->ci_parent));

	printk("keymap release\n");
	clear_bit(keymap->keycode, remote->input->keybit);
	kfree(keymap);
}

static struct configfs_item_operations keymap_ops = {
	.release = keymap_release,
	.show_attribute = item_show,
	.store_attribute = item_store,
};

/* Start the definition of the all of the attributes
 * in a single keymap directory
 */
static struct configfs_attribute *keymap_attrs[] = {
	&item_protocol,
	&item_device,
	&item_command,
	&item_keycode,
	NULL,
};

static struct config_item_type keymap_type = {
	.ct_item_ops = &keymap_ops,
	.ct_attrs	= keymap_attrs,
	.ct_owner	= THIS_MODULE,
};

static struct config_item *make_keymap(struct config_group *group, const char *name)
{
	struct keymap *keymap;

	keymap = kzalloc(sizeof(*keymap), GFP_KERNEL);
	if (!keymap)
		return ERR_PTR(-ENOMEM);

	config_item_init_type_name(&keymap->item, name, &keymap_type);
	return &keymap->item;
}

/*
 * Note that, since no extra work is required on ->drop_item(),
 * no ->drop_item() is provided.
 */
static struct configfs_group_operations remote_group_ops = {
	.make_item = make_keymap,
};

static ssize_t remote_show(struct config_item *item,
					 struct configfs_attribute *attr,
					 char *page)
{
	struct config_group *group = to_config_group(item);
	struct remote *remote  = to_remote(group);
	const char *path;

	if (strcmp(attr->ca_name, "path") == 0) {
		path = kobject_get_path(&remote->input->dev.kobj, GFP_KERNEL);
		strcpy(page, path);
		kfree(path);
		return strlen(page);
	}
	return sprintf(page,
"Map for a specific remote\n"
"Remote signals matching this map will be translated into keyboard/mouse events\n");
}

static void remote_release(struct config_item *item)
{
	struct config_group *group = to_config_group(item);
	struct remote *remote  = to_remote(group);

	printk("remote_release\n");
	input_free_device(remote->input);
	kfree(remote);
}

static struct configfs_item_operations remote_item_ops = {
	.release	= remote_release,
	.show_attribute	= remote_show,
};

static struct configfs_attribute remote_attr_description = {
	.ca_owner = THIS_MODULE,
	.ca_name = "description",
	.ca_mode = S_IRUGO,
};

static struct configfs_attribute remote_attr_path = {
	.ca_owner = THIS_MODULE,
	.ca_name = "path",
	.ca_mode = S_IRUGO,
};

static struct configfs_attribute *remote_attrs[] = {
	&remote_attr_description,
	&remote_attr_path,
	NULL,
};

static struct config_item_type remote_type = {
	.ct_item_ops	= &remote_item_ops,
	.ct_group_ops	= &remote_group_ops,
	.ct_attrs	= remote_attrs,
	.ct_owner	= THIS_MODULE,
};

/* Top level remotes directory for all remotes */

/* Create a new remote group */
static struct config_group *make_remote(struct config_group *parent, const char *name)
{
	struct remote *remote;
	int ret;

	remote = kzalloc(sizeof(*remote), GFP_KERNEL);
	if (!remote)
		return ERR_PTR(-ENOMEM);

	remote->input = input_allocate_device();
	if (!remote->input) {
		ret = -ENOMEM;
		goto free_mem;
	}
	remote->input->id.bustype = BUS_VIRTUAL;
	remote->input->name = name;
	remote->input->phys = "remotes";

	remote->input->evbit[0] = BIT_MASK(EV_KEY);

	ret = input_register_device(remote->input);
	if (ret)
		goto free_input;

	config_group_init_type_name(&remote->group, name, &remote_type);
	return &remote->group;

 free_input:
	input_free_device(remote->input);
 free_mem:
	kfree(remote);
	return ERR_PTR(ret);
}

static ssize_t remotes_show_description(struct config_item *item,
					struct configfs_attribute *attr,
					char *page)
{
	return sprintf(page,
"This subsystem allows the creation of IR remote control maps.\n"
"Maps allow IR signals to be mapped into key strokes or mouse events.\n");
}

static struct configfs_item_operations remotes_item_ops = {
	.show_attribute	= remotes_show_description,
};

static struct configfs_attribute remotes_attr_description = {
	.ca_owner = THIS_MODULE,
	.ca_name = "description",
	.ca_mode = S_IRUGO,
};

static struct configfs_attribute *remotes_attrs[] = {
	&remotes_attr_description,
	NULL,
};

/*
 * Note that, since no extra work is required on ->drop_item(),
 * no ->drop_item() is provided.
 */
static struct configfs_group_operations remotes_group_ops = {
	.make_group	= make_remote,
};

static struct config_item_type remotes_type = {
	.ct_item_ops	= &remotes_item_ops,
	.ct_group_ops	= &remotes_group_ops,
	.ct_attrs	= remotes_attrs,
	.ct_owner	= THIS_MODULE,
};

struct configfs_subsystem input_ir_remotes = {
	.su_group = {
		.cg_item = {
			.ci_namebuf = "remotes",
			.ci_type = &remotes_type,
		},
	},
};

void input_ir_translate(struct input_dev *dev, int protocol, int device, int command)
{
	struct config_item *i, *j;
	struct config_group *g;
	struct remote *remote;
	struct keymap *keymap;

	/* generate the IR format event */
	input_report_ir(dev, IR_PROTOCOL, protocol);
	input_report_ir(dev, IR_DEVICE, device);
	input_report_ir(dev, IR_COMMAND, command);
	input_sync(dev);

    mutex_lock(&input_ir_remotes.su_mutex);

    /* search the translation maps to translate into key stroke */
    list_for_each_entry(i, &input_ir_remotes.su_group.cg_children, ci_entry) {
    	g = to_config_group(i);
        list_for_each_entry(j, &g->cg_children, ci_entry) {
        	keymap = to_keymap(j);
        	if ((keymap->protocol == protocol) && (keymap->device == device)
        			&& (keymap->command == command)) {
				remote = to_remote(g);
				input_report_key(remote->input, keymap->keycode, 1);
				input_sync(remote->input);
        	}
        }
    }
    mutex_unlock(&input_ir_remotes.su_mutex);
}
