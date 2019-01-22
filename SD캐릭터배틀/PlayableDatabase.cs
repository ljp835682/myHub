using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class PlayableDatabase
{
    private object key = new object();

    public static PlayableDatabase inst = null;

    private List<CharaData> equip = null;
    private List<CharaData> box = null;

    private bool[] isHaving = null;

    private int money;
    private bool usingDB;
    private int stage;

    public int Money { get { return money; } } 

    public bool UsingDB
    {
        set { if (value != usingDB) usingDB = value; }
        get { return usingDB; }
    }

    public int Stage
    {
        get { return stage; }
    }

    private PlayableDatabase()
    {
        Init();
    }

    public void Init()
    {
        if (box == null)
        {
            box = new List<CharaData>();
        }

        else
        {
            box.Clear();
        }

        if (equip == null)
        {
            equip = new List<CharaData>();
        }

        else
        {
            equip.Clear();
        }

        if (isHaving == null)
        {
            isHaving = new bool[6];
        }

        isHaving[0] = true;

        for (int i = 1; i < 6; i++)
        {
            isHaving[i] = false;
        }

        money = 2000;
        usingDB = false;
        stage = 1;
    }

    public static void Create()
    {
        if(inst == null)
        {
            inst = new PlayableDatabase();
        }
    }

    public void EquipClear()
    {
        equip.Clear();
    }

    public void BoxClear()
    {
        box.Clear();
    }
   
    public static PlayableDatabase Getinst { get { return inst; } }

    public void AddChara(CharaData data)
    {
        lock (key)
        {
            equip.Add(data);
        }
    }

    public CharaData GetChara(int index)
    {
        lock (key)
        {
            return equip[index];
        }
    }

    public bool GotoBox(int index)
    {
        if (equip.Count == 1 || equip.Count <= index)
        {
            return false;
        }

        else
        {
            int id = equip[index].ID;
            box.Add(equip[index]);
            equip.RemoveAt(index);
            return true;
        }
    }

    public bool GotoEquip(int index)
    {
        if (equip.Count == 3 || box.Count <= index)
        {
            return false;
        }

        else
        {
            equip.Add(box[index]);
            box.RemoveAt(index);
            return true;
        }
    }

    public bool BuyChara(int id,int price)
    {
        if((money - price < 0) || (isHaving[id]))
        {
            return false;
        }

        money -= price;
        isHaving[id] = true;
        box.Add(new CharaData(id));

        return true;
    }

    public CharaData GetBoxChara(int index)
    {
        return box[index];
    }

    public void StageClear()
    {
        if(stage > 12)
        {
            money += 900;
        }

        else if(stage > 6)
        {
            money += 600;
        }

        else
        {
            money += 300;
        }
        
        stage++;
    }

    public bool EquipCure(int price)
    {
        if (money - price < 0)
        {
            return false;
        }

        money -= price;

        for (int i = 0; i < equip.Count; i++)
        {
            if(equip[i].ID != -1)
            {
                equip[i].Hp = equip[i].MaxHp;
            }
        }

        return true;
    }

    public int GetListCount(int state)
    {
        switch (state)
        {
            case 0:
                return equip.Count;
            case 1:
                return box.Count;
            default:
                return -1;
        }
    }
}

