using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class EnemyManager : MonoBehaviour
{
    private List<Chara> charas;
    private int targetCount = 0;

    public int TargetCount
    {
        get { return targetCount; }
    }


    // Use this for initialization
    void Start()
    {
        charas = new List<Chara>();

        for (int i = 0; i < 3; i++)
        {
            charas.Add(transform.Find("CharPos_" + i).gameObject.GetComponent<Chara>());
        }

        NewEnemy();
    }

	// Update is called once per frame
	void Update () {
		
	}

    public int GetCharaCount()
    {
        int Count = 0;

        for (int i = 0; i < 3; i++)
        {
            if(charas[i].ID != -1)
            {
                Count++;
            }
        }

        return Count;
    }

    public int GetCharaID(int index)
    {
        return charas[index].ID;
    }

    public void NewEnemy()
    {
        List<int> list = new List<int>();

        targetCount = 1;

        if (PlayableDatabase.Getinst.Stage > 12)
        {
            targetCount = 3;
        }

        else if(PlayableDatabase.Getinst.Stage > 6)
        {
            targetCount = 2;
        }

        int i = 0;

        while (true)
        {
            bool flag = false;
            int num = Random.Range(6, 12);

            for (int j = 0; j < list.Count; j++)
            {
                if (list[j] == num)
                {
                    flag = true;
                    break;
                }
            }

            if (!flag)
            {
                list.Add(num);
                i++;

                if (i == targetCount)
                {
                    break;
                }
            }
        }

        for (int j = 0; j < targetCount; j++)
        {
            charas[j].NewCharacter(list[j], j * 2);
        }
    }
}
