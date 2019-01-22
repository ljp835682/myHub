using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.UI;

public class KeyPoint : MonoBehaviour {

    public Slider slider;                       //게이지바
    public float maxValue = 3;                      
    private bool isTriggerOn = false;           //충돌중인지 아닌지
    private Animator ani;                       //애니메이션
    private bool isDisappear = false;           //사라진 상태인지 아닌지
    private float anitimer = 15;                //애니메이션 타이머
    private ExitPoint exit;                     //ExitPoint 스크립트의 CountDown 함수를 불러오기위함

    //초기화
    void Start () {
        ani = GetComponent<Animator>();
        exit = GameObject.FindGameObjectWithTag("ExitPoint").GetComponent<ExitPoint>();
    }

    
    void Update () {
        //사라진 상태일때만
        if (isDisappear)
        {
            anitimer -= Time.deltaTime;

            //타이머가 끝나면 안보이게 설정
            if(anitimer <= 0)
            {
                anitimer = 15;
                ani.SetInteger("state", 2);
                gameObject.SetActive(false);
            }
        }
    }

    private void LateUpdate()
    {
        //충돌중이면 슬라이더의 값 상승
        if (isTriggerOn)
        {
            slider.value += Time.deltaTime;
        }
    }

    void OnTriggerEnter(Collider other)
    {
        //플레이어와 충돌중이고 사라지지 않았다면
        if(other.tag == "Player" && !isDisappear)
        {
            isTriggerOn = true;
            slider.gameObject.SetActive(true);
        }
    }

    private void OnTriggerStay(Collider other)
    {
        //플레이어와 충돌중이고 슬라이더가가득찼고 사라진상태가 아니라면
        if (other.tag == "Player" && slider.value >= maxValue && !isDisappear)
        {
            //사라지게 처리
            exit.CountDown();
            ani.SetInteger("state", 1);
            isDisappear = true;
            slider.gameObject.SetActive(false);
        }
    }

    void OnTriggerExit(Collider other)
    {
        //플레이어와 충돌상태가 아니게 되었고 사라진상태가 아니라면
        if (other.tag == "Player" && !isDisappear)
        {
            //슬라이더를 채우지 못했으면 슬라이더값 초기화
            if (slider.value < maxValue)
            {
                slider.value = 0;
            }

            isTriggerOn = false;
            slider.gameObject.SetActive(false);
        }
    }
}
