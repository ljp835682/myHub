using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.SceneManagement;
using UnityEngine.UI;

public class BattleManager : MonoBehaviour
{
    private object key = new object(); 

    private int target = -1;        
    private List<CardData> chosen;

    private List<int> playable_IDs;
    private List<int> enemy_IDs;

    public PlayableManager playable;
    public EnemyManager enemy;
    public FieldChanger field;
    public GameObject shopButton;
    public GameObject startButton;
    public Text roundText;

    public AudioClip trueClip;
    private AudioSource audio;

    private bool isDraw = false;            //카드를 뽑을지 말지 정할 변수
    private bool isRun = false;             //함수를 실행할지 말지 정할 변수
    private bool timerSwitch = false;       //타이머를 킬지 끌지 정할 변수
    private float wait = 1.5f;              //턴마다의 텀
    private int killCount = 0;              //죽인 횟수
    private int deathCount = 0;             //죽은 횟수
    private int oddment = 6;                //남은 턴수

    //카드를 뽑을수 있게 하는 접근자
    public bool Draw
    {
        set
        {
            isDraw = value;

            if(isDraw)
            {
                TrueAudio();
                startButton.SetActive(false);
                shopButton.SetActive(false);
            }
        }
        get { return isDraw; }
    }

    //효과음
    public void TrueAudio()
    {
        audio.Stop();
        audio.clip = trueClip;
        audio.Play();
    }

    //배틀이 시작되는걸 알릴 접근자
    public bool Run
    {
        set { isRun = value; }
        get { return isRun; }
    }

    // Use this for initialization
    void Start()
    {
        audio = GetComponent<AudioSource>();
        playable_IDs = new List<int>();
        enemy_IDs = new List<int>();
        chosen = new List<CardData>();
        deathCount = playable.GetCharaCount();
        roundText.text = PlayableDatabase.Getinst.Stage + " Stage";

        if (PlayableDatabase.Getinst.UsingDB)
        {
            isDraw = false;
        }
    }

    // Update is called once per frame
    void Update()
    {
        if (isRun)
        {
            if (timerSwitch)
            {
                wait -= Time.deltaTime;                     //각 턴마다의 텀

                if (wait <= 0)
                {
                    wait = 1.8f;
                    timerSwitch = false;

                    if(deathCount <= 0)                     //자신의 캐릭터가 전부 죽었을경우
                    {
                        PlayableDatabase.Getinst.Init();  
                        SceneManager.LoadScene("Title");   
                    }

                    else if (oddment == 0)                 //모든 턴 종료                    
                    {
                        chosen.Clear();
                        isRun = false;
                        oddment = 6;

                        if (killCount >= enemy.TargetCount)     //적의 캐릭터를 전부 죽인경우
                        {
                            PlayableDatabase.Getinst.StageClear();
                            roundText.text = PlayableDatabase.Getinst.Stage + " Stage";
                            shopButton.SetActive(true);
                            startButton.SetActive(true);
                            field.ChangeField();
                            enemy.NewEnemy();
                            AddEnemy();
                            killCount = 0;
                            isDraw = false;
                        }

                        else
                        {
                            isDraw = true;
                        }
                    }
                }
            }

            else
            {
                if (oddment > 3)                            //3번의 유저턴
                {
                    Result(playable, enemy, enemy_IDs);
                }

                else                                         //나머지는 컴퓨터턴
                {
                    Result(enemy, playable, playable_IDs);
                }

                timerSwitch = true;

                if (killCount >= enemy.GetComponent<EnemyManager>().TargetCount)
                {
                    oddment = 0;
                }
            }
        }
    }

    public void EnemyDraw()
    {
        int oldsize = chosen.Count;
        while (oldsize + 3 > chosen.Count)
        {
            int id;
            string type;
            id = enemy_IDs[Random.Range(0, enemy_IDs.Count)];

            Chara caster = null;

            for (int j = 0; j < 3; j++)
            {
                if (enemy.transform.Find("CharPos_" + j).GetComponent<Chara>().ID == id)
                {
                    caster = enemy.transform.Find("CharPos_" + j).GetComponent<Chara>();
                    break;
                }
            }
            if (caster.Dead)
            {
                continue;
            }

            switch (Random.Range(0, 3))
            {
                case 0:
                    type = "R";
                    break;
                case 1:
                    type = "G";
                    break;
                case 2:
                    type = "B";
                    break;
                default:
                    type = "R";
                    break;
            }
            chosen.Add(new CardData(id, type));
        }
    }

    public void AddPlayable()
    {
        playable_IDs.Clear();
        for (int i = 0; i < playable.GetCharaCount(); i++)
        {
            if(playable.IsAlive(i))
            {
                playable_IDs.Add(playable.GetCharaID(i));
            }
        }
    }

    public void AddEnemy()
    {
        enemy_IDs.Clear();
        for (int i = 0; i < enemy.GetCharaCount(); i++)
        {
            enemy_IDs.Add(enemy.GetCharaID(i));
        }
    }

    public void Result(EnemyManager atk, PlayableManager def, List<int> def_ID_list)
    {
        Chara caster = null;

        while (true)
        {
            if (chosen.Count == 0)
            {
                oddment = chosen.Count;
                return;
            }

            for (int j = 0; j < 3; j++)        //시전 캐릭터 찾기
            {
                if (atk.transform.Find("CharPos_" + j).GetComponent<Chara>().ID == chosen[0].ID)
                {
                    caster = atk.transform.Find("CharPos_" + j).GetComponent<Chara>();
                    break;
                }
            }

            if (caster.Dead)
            {
                chosen.RemoveAt(0);
                continue;
            }

            else
            {
                break;
            }
        }

        switch (chosen[0].Type)
        {
            case "R":
                Chara reciever = null;

                while (true)
                {
                    reciever = def.transform.Find("CharPos_" + Random.Range(0, 3)).GetComponent<Chara>();
                    if (!reciever.Dead)
                    {
                        break;
                    }
                }

                if (reciever.DoHit(caster.DoAttack()))       //true라면 사망
                {
                    deathCount--;

                    for (int i = 0; i < def_ID_list.Count; i++)
                    {
                        if (def_ID_list[i] == reciever.ID)
                        {
                            def_ID_list.RemoveAt(i);
                        }
                    }
                }

                break;

            case "G":
                caster.DoHeal();
                break;

            case "B":
                caster.DoCharge();
                break;
        }

        chosen.RemoveAt(0);
        oddment = chosen.Count;
    }

    public void Result(PlayableManager atk, EnemyManager def, List<int> def_ID_list)
    {
        Chara caster = null;

        while (true)
        {
            if (chosen.Count == 0)
            {
                oddment = chosen.Count;
                return;
            }

            for (int j = 0; j < 3; j++)        //시전 캐릭터 찾기
            {
                if (atk.transform.Find("CharPos_" + j).GetComponent<Chara>().ID == chosen[0].ID)
                {
                    caster = atk.transform.Find("CharPos_" + j).GetComponent<Chara>();
                    break;
                }
            }

            if (caster.Dead)
            {
                chosen.RemoveAt(0);
                continue;
            }

            else
            {
                break;
            }
        }

        switch (chosen[0].Type)
        {
            case "R":
                Chara reciever = null;

                lock (key)
                {
                    for (int i = 0; i < 3; i++)
                    {
                        if (target == -1)
                        {
                            target = Random.Range(0, 3);
                        }

                        reciever = def.transform.Find("CharPos_" + target).GetComponent<Chara>();

                        if (reciever.Dead)
                        {
                            target++;

                            if (target >= 3)
                            {
                                target = 0;
                            }
                        }

                        else
                        {
                            enemy.transform.GetChild(target).GetComponent<Chara>().Choice = true;
                            break;
                        }
                    }


                    if (reciever.DoHit(caster.DoAttack()))       //true라면 사망
                    {
                        killCount++;
                        target = -1;

                        for (int i = 0; i < def_ID_list.Count; i++)
                        {
                            if (def_ID_list[i] == reciever.ID)
                            {
                                def_ID_list.RemoveAt(i);
                            }
                        }
                    }
                }
                break;

            case "G":
                caster.DoHeal();
                break;

            case "B":
                caster.DoCharge();
                break;
        }

        chosen.RemoveAt(0);
        oddment = chosen.Count;
    }

    public void Cancle(Card card)
    {
        chosen.Remove(card.Data);
    }

    public void Choice(Card card)
    {
        chosen.Add(card.Data);
    }

    public int GetTarget()
    {
        return target;
    }

    public void Cancle()
    {
        lock(key)
        {
            target = -1;
        }
    }

    public void Choice(Chara chara)
    {
        lock (key)
        {
            for (int i = 0; i < enemy_IDs.Count; i++)
            {
                if (chara.GetComponent<Chara>().ID == enemy_IDs[i])
                {
                    target = i;
                    break;
                }
            }
        }
    }

    public int SizeGet()
    {
        return chosen.Count;
    }

    public int GetAliveCharaID()
    {
        return playable_IDs[Random.Range(0, playable_IDs.Count)];
    }
}
