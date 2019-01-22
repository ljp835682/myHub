using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class PlayableManager : MonoBehaviour
{
    private List<Chara> charas;

    // Use this for initialization
    void Start()
    {
        //PlayableDatabase.Create();

        charas = new List<Chara>();

        for (int i = 0; i < 3; i++)
        {
            charas.Add(transform.Find("CharPos_" + i).gameObject.GetComponent<Chara>());
        }

        if(PlayableDatabase.Getinst.UsingDB)
        {
            for (int i = 0; i < PlayableDatabase.Getinst.GetListCount(0); i++)
            {
                charas[i].Data = PlayableDatabase.Getinst.GetChara(i);
                charas[i].LoadCharacter(i * 2);
            }
        }

        else
        {
            charas[0].NewCharacter(0, 0);
            PlayableDatabase.Getinst.AddChara(charas[0].Data);
            PlayableDatabase.Getinst.UsingDB = true;
        }
    }

    public void SetList()
    {
        for (int i = 0; i < PlayableDatabase.Getinst.GetListCount(0); i++)
        {
            charas[i].Data = PlayableDatabase.Getinst.GetChara(i);
        }
    }

    public int GetCharaCount()
    {
        int Count = 0;

        for (int i = 0; i < 3; i++)
        {
            if (charas[i].ID != -1 && !charas[i].Dead)
            {
                Count++;
            }
        }

        return Count;
    }

    public bool IsAlive(int index)
    {
        return !(charas[index].Dead);
    }

    public int GetCharaID(int index)
    {
        return charas[index].ID;
    }

    // Update is called once per frame
    void Update()
    {

    }
}
