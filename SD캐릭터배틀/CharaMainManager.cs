using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.UI;

public class CharaMainManager : MonoBehaviour {

    private Animator ani;
    private GameObject status;

	// Use this for initialization
	void Start () {
        status = transform.GetChild(1).gameObject;
        ani = GetComponent<Animator>();
        ani.SetInteger("state", 0);
    }

    // Update is called once per frame
    void Update()
    {
        Vector2 Pos = Camera.main.ScreenToWorldPoint(Input.mousePosition);
        RaycastHit2D hit = Physics2D.Raycast(Pos, Vector2.zero, 0f);

        if (hit.collider != null)
        {
            if (hit.collider.gameObject == gameObject)
            {
                status.SetActive(true);
            }

            else
            {
                status.SetActive(false);
            }
        }

        else
        {
            status.SetActive(false);
        }
    }

    public void BaseAni()
    {
        ani.SetInteger("state", 0);
    }

    public void AttackAni()
    {
        ani.SetInteger("state", 1);
    }

    public void HitAni()
    {
        Vector2 vetor = new Vector2(transform.position.x, transform.position.y);
        GameObject obj = Instantiate(Resources.Load("Prefabs/Effect/Hit"), vetor, Quaternion.identity) as GameObject;
        obj.transform.parent = gameObject.transform;
        obj.GetComponent<SpriteRenderer>().sortingOrder = gameObject.GetComponent<SpriteRenderer>().sortingOrder + 1;
        ani.SetInteger("state", 2);
        Destroy(obj, 1.3f);
    }

    public void ChargingAni()
    {
        Vector2 vetor = new Vector2(transform.position.x, transform.position.y);
        GameObject obj = Instantiate(Resources.Load("Prefabs/Effect/Charge"), vetor, Quaternion.identity) as GameObject;
        obj.transform.parent = gameObject.transform;
        obj.GetComponent<SpriteRenderer>().sortingOrder = gameObject.GetComponent<SpriteRenderer>().sortingOrder + 1;
        ani.SetInteger("state", 1);
        Destroy(obj, 1.5f);
    }

    public void HealingAni()
    {
        Vector2 vetor = new Vector2(transform.position.x, transform.position.y);
        GameObject obj = Instantiate(Resources.Load("Prefabs/Effect/Heal"), vetor, Quaternion.identity) as GameObject;
        obj.transform.parent = gameObject.transform;
        obj.GetComponent<SpriteRenderer>().sortingOrder = gameObject.GetComponent<SpriteRenderer>().sortingOrder + 1;
        ani.SetInteger("state", 1);
        Destroy(obj, 1.5f);
    }

    public void UIDestroy()
    {
        Transform child = transform.GetChild(0);

        for (int i = child.childCount; i > 0; i--)
        {
            Destroy(child.GetChild(i - 1).gameObject);
        }
    }

    public void ChargeCount(int charge)
    {
        UIDestroy();

        Transform child = transform.GetChild(0);

        int[] uicount = new int[3];

        uicount[0] = charge;

        if(uicount[0] >= 3)
        {
            uicount[2] = uicount[0] / 3;
            uicount[0] = uicount[0] % 3;
        }

        if (uicount[0] >= 2)
        {
            uicount[1] = uicount[0] / 2;
            uicount[0] = uicount[0] % 2;
        }
        

        float y = 0;

        for (int i = 2; i >= 0; i--)
        {
            for (int j = 0; j < uicount[i]; j++)
            {
                Vector2 vector = new Vector2(child.transform.position.x, child.transform.position.y - y);
                y += 0.4f;
                GameObject ui = Instantiate(Resources.Load("Prefabs/Misc/Stack_" + i), vector, Quaternion.identity) as GameObject;
                ui.transform.parent = child.transform;
            }

            if (y == 1.2f)
            {
                break;
            }
        }
    }

    public void Dead()
    {
        Destroy(gameObject, 1.3f);
    }
}
