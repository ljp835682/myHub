using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.SceneManagement;


public class ExitPoint : MonoBehaviour {

    public int stageKeyPointCount = 4;              //현제 스테이지의 연쇠포인트의 갯수
    private static int stageIndex = 1;              //스테이지 번호
    public GameObject endText;                  
    public bool isLastStage;                        //마지막 스테이지인지 확인
    private bool pressflag = false;                 

	// Use this for initialization
	void Start () {
		
	}
	
	// Update is called once per frame
	void Update () {
        if(pressflag)
        {
            if(Input.anyKeyDown)
            {
                SceneManager.LoadScene(stageIndex);
            }
        }
	}

    //열쇠포인트에 닿을경우 호출될것
    public void CountDown()
    {
        stageKeyPointCount--;
    }

    //플레이어가 다음씬으로 갈수잇게해주는 용도
    private void OnTriggerEnter(Collider other)
    {
        if(other.tag == "Player" && stageKeyPointCount == 0)
        {
            if (isLastStage)
            {
                endText.SetActive(true);
                pressflag = true;
                stageIndex = 0;
                GameObject.Find("Enemy").GetComponent<EnemyMovement>().Disable();
            }

            else
            {
                //다음 씬으로
                SceneManager.LoadScene(++stageIndex);
            }
        }
    }
}
