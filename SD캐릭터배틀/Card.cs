using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class CardData
{
    private int card_ID;
    private string card_Type;

    public int ID
    {
        set { card_ID = value; }
        get { return card_ID; }
    }

    public string Type
    {
        set { card_Type = value; }
        get { return card_Type; }
    }

    public CardData(int id, string type)
    {
        card_ID = id;
        card_Type = type;
    }
}

public class Card : MonoBehaviour 
{
    private CardData cardData;
    private GameObject prefab;
    private GameObject frame;
    private bool isChoice;

    public CardData Data
    {
        set { cardData = value; }
        get { return cardData; }
    }

    // Use this for initialization
    void Start () {
        isChoice = false;
    }
	
	// Update is called once per frame
	void Update () {

	}

    public void NewCard()
    {
        Vector2 vetor = new Vector2(transform.position.x, transform.position.y);
        prefab = Instantiate(Resources.Load("Prefabs/Card/Chara" + cardData.ID + "_" + cardData.Type), vetor, Quaternion.identity) as GameObject;
        prefab.transform.parent = gameObject.transform;
        isChoice = false;
    }

    public void Junk()
    {
        Destroy(prefab, 0.5f);
        Destroy(frame, 0.5f);
    }

    public bool Choice
    {
        set
        {
            isChoice = value;

            if (isChoice)
            {
                Vector2 vector = new Vector2(transform.position.x, transform.position.y);
                frame = Instantiate(Resources.Load("Prefabs/Misc/ChoiceFrame_Card"), vector, Quaternion.identity) as GameObject;
                frame.transform.position = new Vector3(frame.transform.position.x, frame.transform.position.y - 0.125f, frame.transform.position.z);
                frame.transform.parent = transform;
            }
            
            else
            {
                if(frame != null)
                {
                    Destroy(frame);
                }
            }
        }

        get { return isChoice; }
    }
}
