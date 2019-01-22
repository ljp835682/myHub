using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class DrawManager : MonoBehaviour {

    private List<Card> cards;
    private GameObject clickObj;
    private int choiceCount = 0;
    private BattleManager bm;

    // Use this for initialization
    void Start () {
        bm = GameObject.FindGameObjectWithTag("BattleManager").GetComponent<BattleManager>();
        cards = new List<Card>();
        for(int i=0;i<5;i++)
        {
            cards.Add(transform.Find("CardPos_" + i).GetComponent<Card>());
        }

        clickObj = null;
        bm.AddEnemy();
        bm.AddPlayable();
    }

    // Update is called once per frame
    void Update()
    {
        switch (Click())
        {
            case 0:
                choiceCount--;
                break;
            case 1:
                choiceCount++;
                break;
            default:
                break;
        }

        if (choiceCount == 3)
        {
            Graveyard();
            bm.EnemyDraw();
            bm.Run = true;
            choiceCount = 0;
        }

        if (bm.Draw)
        {
            Draw();
            bm.Draw = false;
        }
    }

    public int Click()
    {
        if (Input.GetMouseButtonDown(0))
        {
            Vector2 Pos = Camera.main.ScreenToWorldPoint(Input.mousePosition);
            RaycastHit2D hit = Physics2D.Raycast(Pos, Vector2.zero, 0f);

            if (hit.collider != null)
            {
                clickObj = hit.collider.gameObject;

                switch (clickObj.tag)
                {
                    case "Card":

                        if(choiceCount < 3)
                        {
                            clickObj = clickObj.transform.parent.gameObject;
                            if (clickObj.GetComponent<Card>().Choice)
                            {
                                clickObj.GetComponent<Card>().Choice = false;
                                bm.Cancle(clickObj.GetComponent<Card>());
                                return 0;
                            }

                            else
                            {
                                clickObj.GetComponent<Card>().Choice = true;
                                bm.Choice(clickObj.GetComponent<Card>());
                                return 1;
                            }
                        }

                        break;
                    case "Character":
                        clickObj = clickObj.transform.parent.gameObject;
                        if (clickObj.GetComponent<Chara>().Choice)
                        {
                            clickObj.GetComponent<Chara>().Choice = false;
                            bm.Cancle();
                            return 2;
                        }

                        else
                        {
                            clickObj.GetComponent<Chara>().Choice = true;

                            GameObject parents = clickObj.transform.parent.gameObject;

                            for (int i = 0; i < 3; i++)
                            {
                                if (parents.transform.GetChild(i).gameObject != clickObj)
                                {
                                    parents.transform.GetChild(i).GetComponent<Chara>().Choice = false;
                                }
                            }

                            bm.Choice(clickObj.GetComponent<Chara>());
                            return 3;
                        }
                }
            }
        }

        return 4;
    }

    public void Draw()
    {
        string t = "R";
        for (int i = 0; i < 5; i++)
        {
            switch (Random.Range(0, 3))
            {
                case 0:
                    t = "R";
                    break;
                case 1:
                    t = "G";
                    break;
                case 2:
                    t = "B";
                    break;
            }

            cards[i].Data = new CardData(bm.GetAliveCharaID(), t);
            cards[i].NewCard();
        }
    }

    public void Graveyard()
    {
        for (int i=0;i<cards.Count;i++)
        {
            cards[i].Junk();
        }
    }
}
