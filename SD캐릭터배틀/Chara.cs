using System.Collections;
using System.Collections.Generic;
using UnityEngine.UI;
using UnityEngine;

public class CharaData
{
    private int chara_ID = -1;

    private float maxhp = 100;
    private float hp = 0;
    private float damage = 0;
    private float defence = 0;
    private int charge = 1;
    private float heal = 5;
    private bool isDead = true;
    private bool isPlayable = true;

    public int ID
    {
        get { return chara_ID; }
        set { chara_ID = value; }
    }

    public float MaxHp
    {
        get { return maxhp; }
        set { maxhp = value; }
    }

    public float Hp
    {
        get { return hp; }
        set
        {
            hp = value;

            if (hp <= 0)
            {
                isDead = true;
            }
        }
    }

    public bool Dead
    {
        get { return isDead; }
        set { isDead = value; }
    }

    public float Damage
    {
        get { return damage; }
        set { damage = value; }
    }

    public float Defence
    {
        get { return defence; }
        set { defence = value; }
    }

    public int Charge
    {
        get { return charge; }
        set { charge = value; }
    }

    public float Heal
    {
        get { return heal; }
    }

    public bool Playable
    {
        set { isPlayable = value; }
        get { return isPlayable; }
    }

    public CharaData(){}

    public CharaData(int id)
    {
        NewCharacter(id);
    }

    public void NewCharacter(int id)
    {
        chara_ID = id;
        isDead = false;
        charge = 0;

        if (chara_ID >= 6)
        {
            isPlayable = false;
        }

        else
        {
            isPlayable = true;
        }

        switch (chara_ID)
        {
            case 0:
                hp = 30;
                damage = 9.5f;
                defence = 3.5f;
                break;
            case 1:
                hp = 25;
                damage = 11.5f;
                defence = 1.5f;
                break;
            case 2:
                hp = 40;
                damage = 8;
                defence = 6;
                break;
            case 3:
                Hp = 20;
                damage = 15f;
                defence = 1;
                break;
            case 4:
                hp = 10;
                damage = 17f;
                defence = 0;
                break;
            case 5:
                hp = 30;
                damage = 9.5f;
                defence = 3.5f;
                break;

            //--------------------------------

            case 6:
                hp = 40;
                damage = 11;
                defence = 2;
                break;
            case 7:
                hp = 15;
                damage = 16.5f;
                defence = 0;
                break;
            case 8:
                hp = 50;
                damage = 3;
                defence = 10;
                break;
            case 9:
                hp = 25;
                damage = 12.5f;
                defence = 3;
                break;
            case 10:
                hp = 30;
                damage = 12;
                defence = 1;
                break;
            case 11:
                hp = 45;
                damage = 4;
                defence = 9.5f;
                break;
        }

        maxhp = hp;
    }
}

public class Chara : MonoBehaviour {

    private AudioSource audio;
    public AudioClip pisyung;
    public GameObject hpbar;
    private GameObject frame;
    private CharaData chara;
    private bool isChoice = false;
    private float oldhp = 0;

    public CharaData Data
    {
        get { return chara; }
        set { chara = value; }
    }

    public int ID
    {
        get { return chara.ID; }
        set { chara.ID = value; }
    }

    public bool Dead
    {
        get { return chara.Dead; }
        set { chara.Dead = value; }
    }

    public bool Choice
    {
        set
        {
            if(isChoice != value)
            {
                if(!Data.Playable)
                {
                    isChoice = value;

                    if (isChoice)
                    {
                        Vector2 vector = new Vector2(transform.position.x, transform.position.y);
                        frame = Instantiate(Resources.Load("Prefabs/Misc/ChoiceFrame_Chara"), vector, Quaternion.identity) as GameObject;
                        frame.transform.position = new Vector3(frame.transform.position.x, frame.transform.position.y, frame.transform.position.z);
                        frame.transform.parent = transform;
                    }

                    else
                    {
                        if (frame != null)
                        {
                            Destroy(frame);
                        }
                    }
                }
            }
        }

        get { return isChoice; }
    }

    // Use this for initialization
    void Start () {
        chara = new CharaData();
        audio = GetComponent<AudioSource>();
    }
	
	// Update is called once per frame
	void Update ()
    {
        if(chara.Hp != oldhp)
        {
            if(oldhp < chara.Hp)
            {
                oldhp += Time.deltaTime * 50;
                if(oldhp >= chara.Hp)
                {
                    oldhp = chara.Hp;
                }
            }

            else
            {
                oldhp -= Time.deltaTime * 50;
                if (oldhp <= chara.Hp)
                {
                    oldhp = chara.Hp;

                    if(Dead)
                    {
                        DeadAudio();
                    }
                }
            }
        }

        hpbar.GetComponent<Slider>().value = oldhp / chara.MaxHp;

        if (hpbar.GetComponent<Slider>().value <= 0)
        {
            hpbar.SetActive(false);
        }

        else if (hpbar.GetComponent<Slider>().value <= 0.25)
        {
            hpbar.transform.GetChild(1).GetChild(0).GetComponent<Image>().color = Color.red;
        }

        else if (hpbar.GetComponent<Slider>().value <= 0.5)
        {
            hpbar.SetActive(true);
            hpbar.transform.GetChild(1).GetChild(0).GetComponent<Image>().color = Color.yellow;
        }

        else if (hpbar.GetComponent<Slider>().value >= 0.75)
        {
            hpbar.transform.GetChild(1).GetChild(0).GetComponent<Image>().color = Color.green;
        }

        if (hpbar.GetComponent<Slider>().value == 1)
        {
            hpbar.SetActive(true);
        }
    }

    public void DoCharge()
    {
        transform.GetChild(0).GetComponent<CharaMainManager>().ChargingAni();

        if (chara.Charge < 9)
        {
            transform.GetChild(0).GetComponent<CharaMainManager>().ChargeCount(++chara.Charge);
        }
    }

    public void DoHeal()
    {
        oldhp = chara.Hp;
        chara.Hp += chara.Heal;
        if(chara.Hp > chara.MaxHp)
        {
            chara.Hp = chara.MaxHp;
        }
        transform.GetChild(0).GetComponent<CharaMainManager>().HealingAni();
    }

    public float DoAttack()
    {
        float charge = chara.Charge;
        chara.Charge = 0;
        transform.GetChild(0).GetComponent<CharaMainManager>().UIDestroy();
        transform.GetChild(0).GetComponent<CharaMainManager>().AttackAni();

        switch(Random.Range(0, 11))
        {
            case 1:
                return (chara.Damage * 2.5f) + (chara.Damage * (charge * 1.2f));
            default:
                return chara.Damage + (chara.Damage * (charge * 1.2f));
        }
    }

    public bool DoHit(float damage)
    {
        oldhp = chara.Hp;
        float result = damage - chara.Defence;
        result = (result < 0) ? 0 : result;
        chara.Hp -= result;

        transform.GetChild(0).GetComponent<CharaMainManager>().HitAni();

        if (chara.Hp <= 0)
        {
            Choice = false;
            transform.GetChild(0).GetComponent<CharaMainManager>().Dead();
            chara.Dead = true;
            return true;
        }

        return false;
    }

    public void LoadCharacter(int sort)
    {
        oldhp = chara.Hp;

        if(chara.Hp > 0)
        {
            chara.Dead = false;
        }
        
        Vector2 vetor = new Vector2(transform.position.x, transform.position.y);
        GameObject prefab;

        if (chara.Playable)
        {
            prefab = Instantiate(Resources.Load("Prefabs/Playeable/Chara" + chara.ID), vetor, Quaternion.identity) as GameObject;
        }

        else
        {
            prefab = Instantiate(Resources.Load("Prefabs/Enemy/Enemy" + (chara.ID - 6)), vetor, Quaternion.identity) as GameObject;
        }

        prefab.transform.parent = transform;
        prefab.GetComponent<SpriteRenderer>().sortingOrder = sort;
        prefab.GetComponent<CharaMainManager>().ChargeCount(chara.Charge);
    }

    public void NewCharacter(int id, int sort)
    {
        chara.NewCharacter(id);
        oldhp = chara.Hp;

        Vector2 vetor = new Vector2(transform.position.x, transform.position.y);
        GameObject prefab;

        if (chara.Playable)
        {
            prefab = Instantiate(Resources.Load("Prefabs/Playeable/Chara" + chara.ID), vetor, Quaternion.identity) as GameObject;
        }

        else
        {
            prefab = Instantiate(Resources.Load("Prefabs/Enemy/Enemy" + (chara.ID - 6)), vetor, Quaternion.identity) as GameObject;
        }

        prefab.transform.parent = transform;
        prefab.GetComponent<SpriteRenderer>().sortingOrder = sort;
    }

    public void DeadAudio()
    {
        audio.Stop();
        audio.clip = pisyung;
        audio.Play();
    }
}
