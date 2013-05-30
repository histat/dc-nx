
#include "nx.h"
#include "profile.h"
#include "vm_file.h"
#include "profile.fdh"

#define PF_WEAPONS_OFFS		0x38
#define PF_CURWEAPON_OFFS	0x24
#define PF_INVENTORY_OFFS	0xD8
#define PF_TELEPORTER_OFFS	0x158
#define PF_FLAGS_OFFS		0x218

#define MAX_WPN_SLOTS		8
#define MAX_TELE_SLOTS		8

// load savefile #num into the given Profile structure.
bool profile_load(const char *pfname, Profile *file)
{
int i, curweaponslot;
VMFILE *fp;

	stat("Loading profile from %s...", pfname);
	memset(file, 0, sizeof(Profile));
	
	fp = vm_fileopen(pfname, "rb");
	if (!fp)
	{
		staterr("profile_load: unable to open '%s'", pfname);
		return 1;
	}
	
	if (!vm_fverifystring(fp, "Do041220"))
	{
		staterr("profile_load: invalid savegame format: '%s'", pfname);
		vm_fclose(fp);
		return 1;
	}
	
	file->stage = vm_fgetl(fp);
	file->songno = vm_fgetl(fp);
	
	file->px = vm_fgetl(fp);
	file->py = vm_fgetl(fp);
	file->pdir = CVTDir(vm_fgetl(fp));
	
	file->maxhp = vm_fgeti(fp);
	file->num_whimstars = vm_fgeti(fp);
	file->hp = vm_fgeti(fp);
	
	vm_fgeti(fp);						// unknown value
	curweaponslot = vm_fgetl(fp);		// current weapon (slot, not number, converted below)
	vm_fgetl(fp);						// unknown value
	file->equipmask = vm_fgetl(fp);	// equipped items
	
	// load weapons
	vm_fseek(fp, PF_WEAPONS_OFFS, SEEK_SET);
	for(i=0;i<MAX_WPN_SLOTS;i++)
	{
		int type = vm_fgetl(fp);
		if (!type) break;
		
		int level = vm_fgetl(fp);
		int xp = vm_fgetl(fp);
		int maxammo = vm_fgetl(fp);
		int ammo = vm_fgetl(fp);
		
		file->weapons[type].hasWeapon = true;
		file->weapons[type].level = (level - 1);
		file->weapons[type].xp = xp;
		file->weapons[type].ammo = ammo;
		file->weapons[type].maxammo = maxammo;
		
		if (i == curweaponslot)
		{
			file->curWeapon = type;
		}
	}
	
	// load inventory
	file->ninventory = 0;
	vm_fseek(fp, PF_INVENTORY_OFFS, SEEK_SET);
	for(i=0;i<MAX_INVENTORY;i++)
	{
		int item = vm_fgetl(fp);
		if (!item) break;
		
		file->inventory[file->ninventory++] = item;
	}
	
	// load teleporter slots
	file->num_teleslots = 0;
	vm_fseek(fp, PF_TELEPORTER_OFFS, SEEK_SET);
	for(i=0;i<NUM_TELEPORTER_SLOTS;i++)
	{
		int slotno = vm_fgetl(fp);
		int scriptno = vm_fgetl(fp);
		if (slotno == 0) break;
		
		file->teleslots[file->num_teleslots].slotno = slotno;
		file->teleslots[file->num_teleslots].scriptno = scriptno;
		file->num_teleslots++;
	}
	
	// load flags
	vm_fseek(fp, PF_FLAGS_OFFS, SEEK_SET);
	if (!vm_fverifystring(fp, "FLAG"))
	{
		staterr("profile_load: missing 'FLAG' marker");
		vm_fclose(fp);
		return 1;
	}
	
	vm_fresetboolean();
	for(i=0;i<NUM_GAMEFLAGS;i++)
	{
		file->flags[i] = vm_fbooleanread(fp);
	}
	
	vm_fclose(fp);
	return 0;
}


bool profile_save(const char *pfname, Profile *file)
{
VMFILE *fp;
int i;

	//stat("Writing saved game to %s...", pfname);
	fp = vm_fileopen(pfname, "wb");
	if (!fp)
	{
		staterr("profile_save: unable to open %s", pfname);
		return 1;
	}
	
	vm_fputstringnonull("Do041220", fp);
	
	vm_fputl(file->stage, fp);
	vm_fputl(file->songno, fp);
	
	vm_fputl(file->px, fp);
	vm_fputl(file->py, fp);
	vm_fputl((file->pdir == RIGHT) ? 2:0, fp);
	
	vm_fputi(file->maxhp, fp);
	vm_fputi(file->num_whimstars, fp);
	vm_fputi(file->hp, fp);
	
	vm_fseek(fp, 0x2C, SEEK_SET);
	vm_fputi(file->equipmask, fp);
	
	// save weapons
	vm_fseek(fp, PF_WEAPONS_OFFS, SEEK_SET);
	int slotno = 0, curweaponslot = 0;
	
	for(i=0;i<WPN_COUNT;i++)
	{
		if (file->weapons[i].hasWeapon)
		{
			vm_fputl(i, fp);
			vm_fputl(file->weapons[i].level + 1, fp);
			vm_fputl(file->weapons[i].xp, fp);
			vm_fputl(file->weapons[i].maxammo, fp);
			vm_fputl(file->weapons[i].ammo, fp);
			
			if (i == file->curWeapon)
				curweaponslot = slotno;
			
			slotno++;
			if (slotno >= MAX_WPN_SLOTS) break;
		}
	}
	
	if (slotno < MAX_WPN_SLOTS)
		vm_fputl(0, fp);	// 0-type weapon: terminator
	
	// go back and save slot no of current weapon
	vm_fseek(fp, PF_CURWEAPON_OFFS, SEEK_SET);
	vm_fputl(curweaponslot, fp);
	
	// save inventory
	vm_fseek(fp, PF_INVENTORY_OFFS, SEEK_SET);
	for(i=0;i<file->ninventory;i++)
	{
		vm_fputl(file->inventory[i], fp);
	}
	
	vm_fputl(0, fp);
	
	// write teleporter slots
	vm_fseek(fp, PF_TELEPORTER_OFFS, SEEK_SET);
	for(i=0;i<MAX_TELE_SLOTS;i++)
	{
		if (i < file->num_teleslots)
		{
			vm_fputl(file->teleslots[i].slotno, fp);
			vm_fputl(file->teleslots[i].scriptno, fp);
		}
		else
		{
			vm_fputl(0, fp);
			vm_fputl(0, fp);
		}
	}
	
	// write flags
	vm_fseek(fp, PF_FLAGS_OFFS, SEEK_SET);
	vm_fputstringnonull("FLAG", fp);
	
	vm_fresetboolean();
	for(i=0;i<NUM_GAMEFLAGS;i++)
	{
		vm_fbooleanwrite(file->flags[i], fp);
	}
	
	vm_fbooleanflush(fp);
	
	vm_fclose(fp);
	return 0;
}


/*
void c------------------------------() {}
*/

// returns the filename for a save file given it's number
const char *GetProfileName(int num)
{
	if (num == 0)
		return "profile.dat";
	else
		return stprintf("profile%d.dat", num+1);
}

// returns whether the given save file slot exists
bool ProfileExists(int num)
{
	return vmfile_exists(GetProfileName(num));
}

bool AnyProfileExists()
{
	for(int i=0;i<MAX_SAVE_SLOTS;i++)
		if (ProfileExists(i)) return true;
	
	return false;
}



